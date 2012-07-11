/* ambix-jrecord.c -  record and ambix file via jack            -*- c -*-

   Copyright © 2003-2006 Rohan Drape <rd@slavepianos.org>
   Copyright © 2012 IOhannes m zmölnig <zmoelnig@iem.at>.
         Institute of Electronic Music and Acoustics (IEM),
         University of Music and Dramatic Arts, Graz

   This file is based on 'jack.record' from Rohan Drape's "jack-tools" collection

   you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   ambix-jrecord is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <ambix/ambix.h>

#include "jcommon/jack-ringbuffer.h"
#include "jcommon/observe-signal.h"
#include "jcommon/common.h"

struct recorder
{
  int buffer_bytes;
  int buffer_samples;
  int buffer_frames;
  int minimal_frames;
  float timer_seconds;
  int timer_frames;
  int timer_counter;
  float sample_rate;

  float32_t *a_buffer, *e_buffer;

  float *d_buffer;
  float *j_buffer;
  float *u_buffer;
  ambix_fileformat_t file_format;
  ambix_sampleformat_t sample_format;
  ambix_t *sound_file;
  int channels;
  uint32_t a_channels, e_channels;
  jack_port_t **input_port;
  float **in;
  jack_ringbuffer_t *ring_buffer;
  pthread_t disk_thread;
  int pipe[2];
};

#include <sndfile.h>

static ambix_matrix_t*matrix_read(const char*path, ambix_matrix_t*matrix) {
  SF_INFO info;
  uint32_t rows, cols;
  float*data=NULL;
  ambix_matrix_t*mtx=NULL, *result=NULL;
  uint32_t frames;

  memset(&info, 0, sizeof(info));
  SNDFILE*file=sf_open(path, SFM_READ, &info);

  if(!file) {
    fprintf(stderr, "ambix_interleave: matrix open failed '%s'\n", path);
    return NULL;
  }
  rows=info.channels;
  cols=info.frames;
  data=(float*)malloc(rows*cols*sizeof(float));
  frames=sf_readf_float(file, data, cols);
  if(cols!=frames) {
    fprintf(stderr, "ambix_interleave: matrix reading %d frames returned %d\n", frames, cols);
    goto cleanup;
  }

  mtx=ambix_matrix_init(cols, rows, NULL);
  if(mtx && (AMBIX_ERR_SUCCESS==ambix_matrix_fill_data(mtx, data))) {
    uint32_t r, c;
    matrix=ambix_matrix_init(rows, cols, matrix);

    for(r=0; r<rows; r++)
      for(c=0; c<cols; c++)
        matrix->data[r][c]=mtx->data[c][r];
  }

  result=matrix;

  //  fprintf(stderr, "ambix_interleave: matrices not yet supported\n");
  cleanup:
  if(mtx)
    ambix_matrix_destroy(mtx);

  sf_close(file);
  free(data);

  return result;
}

void signal_interleave_to(float32_t *dst, const float32_t **src, uint32_t f, uint32_t c)
{
  uint32_t i, k = 0;
  for(i = 0; i < f; i++) {
    uint32_t j;
    for(j = 0; j < c; j++) {
      dst[k++] = src[j][i];
    }
  }
}

void interleave_split(float32_t*source, uint32_t sourcechannels,
                      uint32_t dst1channels,
                      float32_t*dst1, float32_t*dst2,
                      int64_t frames)
{
  int64_t frame;
  for(frame=0; frame<frames; frame++) {
    uint32_t chan;
    for(chan=0; chan<dst1channels; chan++)
      *dst1++=*source++;
    for(chan=dst1channels; chan<sourcechannels; chan++)
      *dst2++=*source++;
  }
}

void write_to_disk(struct recorder *d, int nframes)
{
  int nsamples = nframes;
  ambix_writef_float32(d->sound_file,
                       d->a_buffer,
                       d->e_buffer,
                       nsamples);
}

void *disk_thread_procedure(void *PTR)
{
  struct recorder *d = (struct recorder *) PTR;
  while(!observe_end_of_process()) {

    /* Wait for data at the ring buffer. */

    int nbytes = d->minimal_frames * sizeof(float) * d->channels;
    nbytes = jack_ringbuffer_wait_for_read(d->ring_buffer, nbytes,
					   d->pipe[0]);

    /* Drop excessive data to not overflow the local buffer. */

    if(nbytes > d->buffer_bytes) {
      eprintf("ambix-jrecord: impossible condition, read space (%d > %d).\n", nbytes, d->buffer_bytes);
      nbytes = d->buffer_bytes;
    }

    /* Read data from the ring buffer. */

    jack_ringbuffer_read(d->ring_buffer,
			 (char *) d->d_buffer,
			 nbytes);

    /* Do write operation.  The sample count *must* be an integral
       number of frames. */

    int nframes = (nbytes / sizeof(float))/ d->channels;
    interleave_split(d->d_buffer, d->channels,
                     d->a_channels,
                     d->a_buffer, d->e_buffer,
                     nframes);

    write_to_disk(d, nframes);

    /* Handle timer */

    d->timer_counter += nframes;
    if(d->timer_frames > 0 && d->timer_counter >= d->timer_frames) {
      return NULL;
    }
  }
  return NULL;
}

/* Write data from the JACK input ports to the ring buffer.  If the
   disk thread is late, ie. the ring buffer is full, print an error
   and halt the client.  */

int process(jack_nframes_t nframes, void *PTR)
{
  struct recorder *d = (struct recorder *) PTR;
  int nsamples = nframes * d->channels;
  int nbytes = nsamples * sizeof(float);

  /* Get port data buffers. */

  int i;
  for(i = 0; i < d->channels; i++) {
    d->in[i] = (float *) jack_port_get_buffer(d->input_port[i], nframes);
  }

  /* Check period size is workable. If the buffer is large, ie 4096
     frames, this should never be of practical concern. */

  if(nbytes >= d->buffer_bytes) {
    eprintf("ambix-jrecord: period size exceeds limit\n");
    FAILURE;
    return 1;
  }

  /* Check that there is adequate space in the ringbuffer. */

  int space = (int) jack_ringbuffer_write_space(d->ring_buffer);
  if(space < nbytes) {
    eprintf("ambix-jrecord: overflow error, %d > %d\n", nbytes, space);
    FAILURE;
    return 1;
  }

  /* Interleave input to buffer and copy into ringbuffer. */

  signal_interleave_to(d->j_buffer,
		       (const float **)d->in,
		       nframes,
		       d->channels);
  int err = jack_ringbuffer_write(d->ring_buffer,
				  (char *) d->j_buffer,
				  (size_t) nbytes);
  if(err != nbytes) {
    eprintf("ambix-jrecord: error writing to ringbuffer, %d != %d\n",
	    err, nbytes);
    FAILURE;
    return 1;
  }

  /* Poke the disk thread to indicate data is on the ring buffer. */

  char b = 1;
  xwrite(d->pipe[1], &b, 1);

  return 0;
}

void usage(const char*name)
{
  eprintf("Usage: %s [ options ] sound-file\n", name);
  eprintf("Record an ambix file via JACK\n");
  eprintf("\n");
  eprintf("Options:\n");
  eprintf("    -O N : ambisonics order (default=1).\n");
  eprintf("    -X s : sound-file holding adaptor matrix matrix to reconstruct full ambisonics set (forces AMBIX_EXTENDED format).\n");
  eprintf("    -x N : Number of non-ambisonics ('extra') channels (forces AMBIX_EXTENDED format).\n");

  eprintf("    -b N : Ring buffer size in frames (default=4096).\n");
  // LATER: allow user to specify the sample-format
  //  eprintf("    -f N : File format (default=0x10006).\n");
  eprintf("    -m N : Minimal disk read size in frames (default=32).\n");
  eprintf("    -t N : Set a timer to record for N seconds (default=-1).\n");
  eprintf("    -V : Print version information.\n");
  eprintf("    -h : Print this help.\n");
  eprintf("\n");

#ifdef PACKAGE_BUGREPORT
  eprintf("Report bugs to: %s\n\n", PACKAGE_BUGREPORT);
#endif
#ifdef PACKAGE_URL
  eprintf("Home page: %s\n", PACKAGE_URL);
#endif

  FAILURE;
}

void version(const char*name)
{
#ifdef PACKAGE_VERSION
  eprintf("%s %s\n", name, PACKAGE_VERSION);
#endif
  eprintf("\n");
  eprintf("Copyright (C) 2003-2010 Rohan Drape\n");
  eprintf("Copyright (C) 2012 Institute of Electronic Music and Acoustics (IEM), University of Music and Dramatic Arts (KUG), Graz, Austria.\n");
  eprintf("\n");
  eprintf("License GPLv2: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>\n");
  eprintf("This is free software: you are free to change and redistribute it.\n");
  eprintf("There is NO WARRANTY, to the extent permitted by law.\n");
  eprintf("\n");
  eprintf("Written by IOhannes m zmoelnig <zmoelnig@iem.at>, based on jack.record by Rohan Drape\n");
  FAILURE;
}

int main(int argc, char *argv[])
{
  const char*filename = "jtest.caf";
  const char*myname=argv[0];
  observe_signals ();
  struct recorder d;

  ambix_matrix_t*matrix=NULL;
  int32_t order = -1;

  d.buffer_frames = 4096;
  d.minimal_frames = 32;
  d.channels = 2;
  d.timer_seconds = -1.0;
  d.timer_counter = 0;
  d.sample_format = AMBIX_SAMPLEFORMAT_FLOAT32;
  d.file_format   = AMBIX_BASIC;
  int c;
  while((c = getopt(argc, argv, "hVx:X:O:b:fhm:n:t:")) != -1) {
    switch(c) {
    case 'x':
      d.e_channels = (int) strtol(optarg, NULL, 0);
      d.file_format   = AMBIX_EXTENDED;
      break;
    case 'X':
      matrix=matrix_read(optarg, matrix);
      if(!matrix) {
        eprintf("%s: couldn't read matrix-file '%s'\n", myname, optarg);
        FAILURE;
      }
      d.file_format   = AMBIX_EXTENDED;
      break;
    case 'O':
      order = (uint32_t) strtol(optarg, NULL, 0);
      break;

    case 'b':
      d.buffer_frames = (int) strtol(optarg, NULL, 0);
      break;
#if 0
    case 'f':
      d.file_format = (int) strtol(optarg, NULL, 0);
      break;
#endif
    case 'V':
      version (myname);
      break;
    case 'h':
      usage (myname);
      break;
    case 'm':
      d.minimal_frames = (int) strtol(optarg, NULL, 0);
      break;
    case 't':
      d.timer_seconds = (float) strtod(optarg, NULL);
      break;
    default:
      eprintf("%s: illegal option, %c\n", myname, c);
      usage (myname);
      break;
    }
  }

  if(optind == argc - 1) {
    filename=argv[optind];
  } else {
    eprintf("opening default file '%s'\n", filename);
    //usage (myname);
  }

  /* Allocate channel based data. */
  if(matrix) {
    if(order<0) {
      d.a_channels = matrix->cols;
    } else {
      if(ambix_order2channels(order) != matrix->rows) {
        eprintf("%s: ambisonics order:%d cannot use [%dx%d] adaptor matrix.\n", myname, order, matrix->rows, matrix->cols);
        FAILURE;
      }
      d.a_channels = matrix->cols;
    }
  } else {
    if(order<0)
      order=1;

    d.a_channels=ambix_order2channels(order);
  }

  switch(d.file_format) {
  case AMBIX_BASIC:
    //d.a_channels;
    d.e_channels=0;
    break;
  case AMBIX_EXTENDED:
    //d.a_channels;
    //d.e_channels;
    break;
  case AMBIX_NONE: default:
    d.a_channels=0;
    //d.e_channels;
  }
  d.channels = d.a_channels+d.e_channels;


  if(d.channels < 1) {
    eprintf("%s: illegal number of channels: %d\n", myname, d.channels);
    FAILURE;
  }
  d.in = (float**)xmalloc(d.channels * sizeof(float *));
  d.input_port = (jack_port_t**)xmalloc(d.channels * sizeof(jack_port_t *));

  /* Connect to JACK. */

  jack_client_t *client = jack_client_unique_("ambix-jrecord");
  jack_set_error_function(jack_client_minimal_error_handler);
  jack_on_shutdown(client, jack_client_minimal_shutdown_handler, 0);
  jack_set_process_callback(client, process, &d);
  d.sample_rate = jack_get_sample_rate(client);

  /* Setup timer. */

  if(d.timer_seconds < 0.0) {
    d.timer_frames = -1;
  } else {
    d.timer_frames = d.timer_seconds * d.sample_rate;
  }

  /* Create sound file. */

  ambix_info_t sfinfo;
  memset(&sfinfo, 0, sizeof(sfinfo));
  sfinfo.samplerate = (int) d.sample_rate;
  sfinfo.frames = 0;
  sfinfo.fileformat = d.file_format;

  sfinfo.ambichannels  = d.a_channels;
  sfinfo.extrachannels = d.e_channels;

  d.sound_file = ambix_open(filename, AMBIX_WRITE, &sfinfo);

  if(matrix) {
    ambix_err_t aerr = ambix_set_adaptormatrix(d.sound_file, matrix);
    if(AMBIX_ERR_SUCCESS != aerr) {
      eprintf("setting [%dx%d] matrix returned %d.\n", matrix->rows, matrix->cols, aerr);
      FAILURE;
    }
  }

  /* Allocate buffers. */

  d.buffer_samples = d.buffer_frames * d.channels;
  d.buffer_bytes = d.buffer_samples * sizeof(float);

  d.a_buffer = (float32_t*)xmalloc(d.buffer_frames * d.a_channels * sizeof(float32_t));
  d.e_buffer = (float32_t*)xmalloc(d.buffer_frames * d.e_channels * sizeof(float32_t));

  d.d_buffer = (float*)xmalloc(d.buffer_bytes);
  d.j_buffer = (float*)xmalloc(d.buffer_bytes);
  d.u_buffer = (float*)xmalloc(d.buffer_bytes);
  d.ring_buffer = jack_ringbuffer_create(d.buffer_bytes);

  /* Create communication pipe. */

  xpipe(d.pipe);

  /* Start disk thread. */

  pthread_create (&(d.disk_thread),
		  NULL,
		  disk_thread_procedure,
		  &d);

  /* Create input ports and activate client. */

#if 0
  jack_port_make_standard(client, d.input_port, d.channels, false);
  jack_client_activate(client);
#else
  do {
    int i=0, a, e;
    const char*format=(sfinfo.fileformat == AMBIX_BASIC)?"ACN_%d":"ambisonics_%d";
    const int a_offset=(sfinfo.fileformat == AMBIX_BASIC)?0:1;
    for(a=0; a<d.a_channels; a++) {
      d.input_port[i] = _jack_port_register(client, JackPortIsInput, format, a+a_offset);
      i++;
    }
    for(e=0; e<d.e_channels; e++) {
      d.input_port[i] = _jack_port_register(client, JackPortIsInput, "in_%d", e+1);
      i++;
    }
  } while(0);

  if(jack_activate(client)) {
    eprintf("jack_activate() failed\n");
    FAILURE;
  }
#endif

  /* Wait for disk thread to end, which it does when it reaches the
     end of the file or is interrupted. */

  pthread_join(d.disk_thread, NULL);

  /* Close sound file, free ring buffer, close JACK connection, close
     pipe, free data buffers, indicate success. */

  jack_client_close(client);
  ambix_close(d.sound_file);
  jack_ringbuffer_free(d.ring_buffer);
  close(d.pipe[0]);
  close(d.pipe[1]);

  free(d.a_buffer);
  free(d.e_buffer);

  free(d.d_buffer);
  free(d.j_buffer);
  free(d.u_buffer);
  free(d.in);
  free(d.input_port);
  if(matrix)ambix_matrix_destroy(matrix);
  return EXIT_SUCCESS;
}
