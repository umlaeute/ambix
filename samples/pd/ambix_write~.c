/* ambix_write~ -  write AMBIsonics eXchange files in Pd        -*- c -*-

 Copyright © 1997-1999 Miller Puckette <msp@ucsd.edu>.
 Copyright © 2012-2014 IOhannes m zmölnig <zmoelnig@iem.at>.
 Copyright © 2016, Matthias Kronlachner
 Institute of Electronic Music and Acoustics (IEM),
 University of Music and Dramatic Arts, Graz

 This file is part of libambix

 libambix is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as
 published by the Free Software Foundation; either version 2.1 of
 the License, or (at your option) any later version.

 libambix is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this program; if not, see <http://www.gnu.org/licenses/>.

 [ambix_write~] is based on Pd's [writesf~], which is released under a
 3-clause BSD-License ("Standard Improved BSD License")

*/

#ifndef _WIN32
# include <unistd.h>
# include <fcntl.h>
#endif

#include <pthread.h>

#ifdef _WIN32
# include <io.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <m_pd.h>
#include <ambix/ambix.h>

#include "winhacks.h"

#define MAXSFCHANS 64
#define DEFAULTVECSIZE 128

#define READFRAMES 16384
#define WRITFRAMES 16384
#define DEFBUFPERCHAN 65536
#define MINBUFSIZE (4 * READFRAMES)
#define MAXBUFSIZE 4194304

#define REQUEST_NOTHING 0
#define REQUEST_OPEN 1
#define REQUEST_CLOSE 2
#define REQUEST_QUIT 3
#define REQUEST_BUSY 4

#define STATE_IDLE 0
#define STATE_STARTUP 1
#define STATE_STREAM 2

/******************** soundfile access routines **********************/
static int ambixwrite_argparse(void *obj, int *p_argc, t_atom **p_argv,
                               t_symbol **p_filesym,
                               ambix_fileformat_t *p_fileformat, ambix_sampleformat_t *p_sampleformat, t_float *p_rate)
{
  int argc = *p_argc;
  t_atom *argv = *p_argv;
  ambix_fileformat_t fileformat=AMBIX_EXTENDED;
  ambix_sampleformat_t sampleformat=AMBIX_SAMPLEFORMAT_PCM24;
  t_symbol *filesym;
  t_float rate = -1;

  while (argc > 0 && argv->a_type == A_SYMBOL &&
         *argv->a_w.w_symbol->s_name == '-')
    {
      const char *flag = argv->a_w.w_symbol->s_name + 1;
      if(0) {
#if 0
      } else if (!strcmp(flag, "nframes")) {
        if (argc < 2 || argv[1].a_type != A_FLOAT ||
            ((nframes = argv[1].a_w.w_float) < 0))
          goto usage;
        argc -= 2; argv += 2;
#endif
      } else if (!strcmp(flag, "bytes")) {
        if (argc < 2 || argv[1].a_type != A_FLOAT)
          goto usage;
        switch(atom_getint(argv+1)) {
        case 2: sampleformat=AMBIX_SAMPLEFORMAT_PCM16;   break;
        case 3: sampleformat=AMBIX_SAMPLEFORMAT_PCM24;   break;
        case 4: sampleformat=AMBIX_SAMPLEFORMAT_FLOAT32; break;
        default:
          goto usage;
        }
        argc -= 2; argv += 2;
      } else if (!strcmp(flag, "r") || !strcmp(flag, "rate")) {
        if (argc < 2 || argv[1].a_type != A_FLOAT ||
            ((rate = argv[1].a_w.w_float) <= 0))
          goto usage;
        argc -= 2; argv += 2;
      }
      else goto usage;
    }
  if (!argc || argv->a_type != A_SYMBOL)
    goto usage;
  filesym = argv->a_w.w_symbol;

  argc--; argv++;

  *p_argc = argc;
  *p_argv = argv;
  if(p_filesym)
    *p_filesym = filesym;
  if(p_fileformat)
    *p_fileformat = fileformat;
  if(p_sampleformat)
    *p_sampleformat = sampleformat;
  if(p_rate)
    *p_rate = rate;
  return (0);
 usage:
  return (-1);
}

/* takes sample-blocks (per channel) and interleaves them */
/* to be used in perform() to get Pd-channels into the fifo */
static void interleave_samples(t_sample**invecs, uint32_t channels, t_sample*outbuf, uint32_t frames) {
  uint32_t f, c;
  for(f=0; f<frames; f++) {
    for(c=0; c<channels; c++) {
      *outbuf++=invecs[c][f];
    }
  }
}

/* split a block of (chan1+chan2) interleaved  samples into 2 blocks of chan1 interleaved samples and chan2 interleaved samples */
/* to be used in the writer thread to get ambix compatible data from the fifo */
static void split_samples(t_sample*input, uint32_t frames,
                          float32_t*buf1, uint32_t chan1,
                          float32_t*buf2, uint32_t chan2) {
  uint32_t f=0;
  uint32_t chan=0;
  for(f=0; f<frames; f++) {
    for(chan=0; chan<chan1; chan++)
      *buf1++=*input++;
    for(chan=0; chan<chan2; chan++)
      *buf2++=*input++;
  }
}

/************************* ambix_write object ******************************/

/* [ambix_write~] uses the Posix threads package; for the moment we're Linux
   only although this should be portable to the other platforms.

   Each instance of [ambix_write~] owns a "child" thread for doing the unix (MSW?) file
   reading.  The parent thread signals the child each time:
   (1) a file wants opening or closing;
   (2) we've eaten another 1/16 of the shared buffer (so that the
   child thread should check if it's time to read some more.)
   The child signals the parent whenever a read has completed.  Signalling
   is done by setting "conditions" and putting data in mutex-controlled common
   areas.
*/

typedef struct _ambix_write
{
  t_object x_obj;
  t_canvas *x_canvas;
  t_sample *x_buf;                        /* soundfile buffer (layout: A0 A1 ... An B0 B1 ... Bn ... Xn */
  int       x_bufsize;                    /* buffer size in bytes */
  int       x_bufframes;                  /* buffer size in frames */

  int x_noutlets;                         /* number of audio outlets */
  t_sample *(x_invec[MAXSFCHANS]);       /* audio vectors */
  int x_vecsize;                          /* vector size for transfers */

  int x_state;                            /* opened, running, or idle */
  int x_requestcode;      /* pending request from parent to I/O thread */

  t_float x_insamplerate;   /* sample rate of input signal if known */

  /* parameters to communicate with subthread */
  const char *x_filename;       /* file to open (string is permanently allocated) */
  ambix_fileformat_t x_fileformat; /* extended or basic */
  ambix_sampleformat_t x_sampleformat; /* 16, 24 or 32 bit */
  uint32_t x_ambichannels; /* number of ambisonics channels in soundfile */
  uint32_t x_extrachannels; /* number of extra channels in soundfile */
  ambix_matrix_t*x_matrix;
  t_float x_samplerate;     /* sample rate of soundfile */

  int x_fileerror;        /* slot for "errno" return */


  long x_onsetframes;     /* number of sample frames to skip */
  long x_bytelimit;       /* max number of data bytes to read */


  int x_fifosize;         /* buffer size appropriately rounded down */
  int x_fifohead;         /* index of next byte to get from file */
  int x_fifotail;         /* index of next byte the ugen will read */

  int x_eof;              /* true if fifohead has stopped changing */

  int x_sigcountdown;     /* counter for signalling child for more data */
  int x_sigperiod;        /* number of ticks per signal */

  pthread_mutex_t x_mutex;
  pthread_cond_t x_requestcondition;
  pthread_cond_t x_answercondition;
  pthread_t x_childthread;

  t_float x_f;            /* ambix_write~ only; scalar for signal inlet */
} t_ambix_write;


/******************************* ambix_write *******************/

static t_class *ambix_write_class;

/************** the child thread which performs file I/O ***********/

static void *ambix_write_child_main(void *zz) {
  t_ambix_write *x = (t_ambix_write*)zz;
  ambix_t*ambix=NULL;
  pthread_mutex_lock(&x->x_mutex);
  while (1) {
    if (x->x_requestcode == REQUEST_NOTHING) {
      pthread_cond_signal(&x->x_answercondition);
      pthread_cond_wait(&x->x_requestcondition, &x->x_mutex);
    } else if (x->x_requestcode == REQUEST_OPEN) {
      int sysrtn, writeframes;
      ambix_info_t ainfo;

      /* copy file stuff out of the data structure so we can
         relinquish the mutex while we're in open_soundfile(). */
      int64_t onsetframes = x->x_onsetframes;

      ambix_fileformat_t fileformat = x->x_fileformat;
	  
	  ambix_sampleformat_t sampleformat = x->x_sampleformat;

      uint32_t ambichannels  = x->x_ambichannels;
      uint32_t xtrachannels  = x->x_extrachannels;
      int localfifosize = x->x_fifosize;

      float32_t*ambibuf = NULL;
      float32_t*xtrabuf = NULL;

      double samplerate = x->x_samplerate;

      ambix_matrix_t*matrix=NULL;

      char *filename = strndup(x->x_filename, MAXPDSTRING);

      if(x->x_matrix)
        matrix=ambix_matrix_copy(x->x_matrix, matrix);


      /* alter the request code so that an ensuing "open" will get
         noticed. */
      x->x_requestcode = REQUEST_BUSY;
      x->x_fileerror = 0;

      /* open the soundfile with the mutex unlocked */
      pthread_mutex_unlock(&x->x_mutex);

      memset(&ainfo, 0, sizeof(ainfo));

      ainfo.fileformat=fileformat;

      ainfo.ambichannels=ambichannels;
      ainfo.extrachannels=xtrachannels;

      ainfo.samplerate=samplerate;
      ainfo.sampleformat=sampleformat;
      /* if there's already a file open, close it.  This
         should never happen since ambix_write_open() calls stop if
         needed and then waits until we're idle. */
      if (ambix)
        ambix_close(ambix);
      ambix=ambix_open(filename, AMBIX_WRITE, &ainfo);

      free(filename);

      if(matrix) {
        if(ambix)
          ambix_set_adaptormatrix(ambix, matrix);

        ambix_matrix_destroy(matrix);
        matrix=NULL;
      }

      if(ambix && onsetframes) {
        ambix_seek(ambix, onsetframes, SEEK_SET);
      }

      if(ambix) {
        ambibuf = (float32_t*)calloc(localfifosize*ambichannels, sizeof(float32_t));
        xtrabuf = (float32_t*)calloc(localfifosize*xtrachannels, sizeof(float32_t));
      }

      pthread_mutex_lock(&x->x_mutex);
      if(NULL==ambix) {
        x->x_eof = 1;
        x->x_fileerror = errno;
        x->x_requestcode = REQUEST_NOTHING;
        continue;
      }

      /* check if another request has been made; if so, field it */
      if (x->x_requestcode != REQUEST_BUSY)
        continue;

      x->x_fifotail = 0;

      /* in a loop, wait for the fifo to have data and write it
         to disk */
      while (x->x_requestcode == REQUEST_BUSY ||
             (x->x_requestcode == REQUEST_CLOSE &&
              x->x_fifohead != x->x_fifotail)) {
        int fifosize = x->x_fifosize, fifotail;
        t_sample*buf = x->x_buf;

        /* if the head is < the tail, we can immediately write
           from tail to end of fifo to disk; otherwise we hold off
           writing until there are at least WRITESIZE bytes in the
           buffer */
        if (x->x_fifohead < x->x_fifotail ||
            x->x_fifohead >= x->x_fifotail + WRITFRAMES
            || (x->x_requestcode == REQUEST_CLOSE &&
                x->x_fifohead != x->x_fifotail)) {
          writeframes = (x->x_fifohead < x->x_fifotail ? fifosize : x->x_fifohead) - x->x_fifotail;
          if (writeframes > READFRAMES)
            writeframes = READFRAMES;
        } else {
          pthread_cond_signal(&x->x_answercondition);
          pthread_cond_wait(&x->x_requestcondition,
                            &x->x_mutex);
          continue;
        }
        fifotail = x->x_fifotail;
        pthread_mutex_unlock(&x->x_mutex);
        if(localfifosize<fifosize) {
          free(ambibuf); free(xtrabuf);
          localfifosize=fifosize;
          ambibuf = (float32_t*)calloc(localfifosize*ambichannels, sizeof(float32_t));
          xtrabuf = (float32_t*)calloc(localfifosize*xtrachannels, sizeof(float32_t));
        }
        split_samples(buf+fifotail*(ambichannels+xtrachannels), writeframes,
                      ambibuf, ambichannels,
                      xtrabuf, xtrachannels);

        sysrtn = ambix_writef_float32(ambix,
                                      ambibuf,
                                      xtrabuf,
                                      writeframes);
        pthread_mutex_lock(&x->x_mutex);
        if (x->x_requestcode != REQUEST_BUSY &&
            x->x_requestcode != REQUEST_CLOSE)
          break;

        if (sysrtn < writeframes) {
          x->x_fileerror = errno;
          break;
        } else {
          x->x_fifotail += sysrtn;

          if (x->x_fifotail == fifosize)
            x->x_fifotail = 0;
        }
        /* signal parent in case it's waiting for data */
        pthread_cond_signal(&x->x_answercondition);
      }
      free(ambibuf);free(xtrabuf);
    } else if (x->x_requestcode == REQUEST_CLOSE ||
             x->x_requestcode == REQUEST_QUIT) {
      int quit = (x->x_requestcode == REQUEST_QUIT);
      if (ambix) {
        pthread_mutex_unlock(&x->x_mutex);

        ambix_close(ambix);
        ambix=NULL;

        pthread_mutex_lock(&x->x_mutex);
      }
      x->x_requestcode = REQUEST_NOTHING;
      pthread_cond_signal(&x->x_answercondition);
      if (quit)
        break;
    } else {
    }
  }
  pthread_mutex_unlock(&x->x_mutex);
  return (0);
}

/******** the object proper runs in the calling (parent) thread ****/

//static void *ambix_write_new(t_floatarg fnchannels, t_floatarg fbufsize) {
static void *ambix_write_new(t_symbol*s, int argc, t_atom*argv) {
  int achannels=0, xchannels=0, bufframes=-1, bufsize=0;
  int have_x=0, limiting=0;
  t_ambix_write *x;
  int nchannels, i;

  t_sample*buf;

  switch(argc) {
  case 0:
    achannels=4;
    break;
  case 1:
    achannels=atom_getint(argv+0);
    break;
  case 2:
    achannels=atom_getint(argv+0);
    xchannels=atom_getint(argv+1);
    have_x=1;
    break;
  case 3:
    achannels=atom_getint(argv+0);
    xchannels=atom_getint(argv+1);
    have_x=1;
    bufframes  =atom_getint(argv+2);
    break;
  default:
    pd_error(0, "usage: [ambix_write~ <ambichannels> <extrachannels> <buffersize>]");
    return NULL;
  }

  if(achannels+xchannels>MAXSFCHANS) {
    /* ouch, user requested too much! */
    if(achannels>MAXSFCHANS) {
      achannels=MAXSFCHANS;
      xchannels=0;
    } else {
      xchannels=MAXSFCHANS-achannels;
    }
    limiting=1;
  }
  nchannels=achannels+xchannels;

  if (bufframes <= 0) bufframes = DEFBUFPERCHAN;
  else if (bufframes < MINBUFSIZE)
    bufframes = MINBUFSIZE;
  else if (bufframes > MAXBUFSIZE)
    bufframes = MAXBUFSIZE;

  bufsize=bufframes*nchannels;

  buf = (t_sample*)getbytes(bufsize*sizeof(t_sample));
  if (!buf) return (0);

  x = (t_ambix_write *)pd_new(ambix_write_class);

  if(limiting) {
    pd_error(x, "limiting to %d ambisonics channels and %d extra channels", achannels, xchannels);
    if(have_x) {
      /* FIXXME: remove 'have_x' variable */
    }
  }

  for (i = 1; i < nchannels; i++)
    inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_signal, &s_signal);

  x->x_f = 0;

  x->x_ambichannels = achannels;
  x->x_extrachannels = xchannels;

  x->x_matrix=NULL;
  x->x_canvas = canvas_getcurrent();

  pthread_mutex_init(&x->x_mutex, 0);
  pthread_cond_init(&x->x_requestcondition, 0);
  pthread_cond_init(&x->x_answercondition, 0);

  pthread_mutex_lock(&x->x_mutex);
  x->x_vecsize = DEFAULTVECSIZE;
  x->x_insamplerate = x->x_samplerate = 0;
  x->x_state = STATE_IDLE;

  x->x_buf = buf;
  x->x_bufsize = bufsize;
  x->x_bufframes = bufframes;

  x->x_fifosize = x->x_fifohead = x->x_fifotail = x->x_requestcode = 0;
  pthread_mutex_unlock(&x->x_mutex);

  pthread_create(&x->x_childthread, 0, ambix_write_child_main, x);
  return (x);
}

static t_int *ambix_write_perform(t_int *w) {
  t_ambix_write *x = (t_ambix_write *)(w[1]);
  uint32_t achannels = x->x_ambichannels, xchannels=x->x_extrachannels;
  int vecsize = x->x_vecsize;
  uint32_t channels=achannels+xchannels;
  if (x->x_state == STATE_STREAM) {
    pthread_mutex_lock(&x->x_mutex);
    while (x->x_fifotail > x->x_fifohead &&
           x->x_fifotail < x->x_fifohead + vecsize + 1) {
      pthread_cond_signal(&x->x_requestcondition);
      pthread_cond_wait(&x->x_answercondition, &x->x_mutex);
    }
    interleave_samples(x->x_invec, channels,
                       x->x_buf+(x->x_fifohead*channels),
                       vecsize);

    x->x_fifohead += vecsize;
    if (x->x_fifohead >= x->x_fifosize)
      x->x_fifohead = 0;
    if ((--x->x_sigcountdown) <= 0) {
      pthread_cond_signal(&x->x_requestcondition);
      x->x_sigcountdown = x->x_sigperiod;
    }
    pthread_mutex_unlock(&x->x_mutex);
  }
  return (w+2);
}

static void ambix_write_start(t_ambix_write *x) {
  /* start making output.  If we're in the "startup" state change
     to the "running" state. */
  pthread_mutex_lock(&x->x_mutex);
  if (x->x_state == STATE_STARTUP) {
    x->x_state = STATE_STREAM;
    pthread_mutex_unlock(&x->x_mutex);
  } else {
    pthread_mutex_unlock(&x->x_mutex);
    pd_error(x, "ambix_write: start requested with no prior 'open'");
  }
}

static void ambix_write_stop(t_ambix_write *x) {
  /* LATER rethink whether you need the mutex just to set a Svariable? */
  pthread_mutex_lock(&x->x_mutex);
  x->x_state = STATE_IDLE;
  x->x_requestcode = REQUEST_CLOSE;
  pthread_cond_signal(&x->x_requestcondition);
  pthread_mutex_unlock(&x->x_mutex);
}

/* open method.  Called as: open [args] filename with args as in
   ambixwrite_argparse().
*/
static void ambix_write_open(t_ambix_write *x, t_symbol *s, int argc, t_atom *argv) {
  t_symbol *filesym;
  t_float samplerate;

  ambix_fileformat_t fileformat;
  ambix_sampleformat_t sampleformat;


  if (x->x_state != STATE_IDLE) {
    ambix_write_stop(x);
  }

  if (ambixwrite_argparse(x, &argc, &argv,
                               &filesym, &fileformat, &sampleformat, &samplerate)) {
    pd_error(x,
             "ambix_write~: usage: open [-bytes [234]] [-rate ####] filename");
    return;
  }

  if (argc)
    pd_error(x, "extra argument(s) to ambix_write~: ignored");

  pthread_mutex_lock(&x->x_mutex);
  while (x->x_requestcode != REQUEST_NOTHING) {
    pthread_cond_signal(&x->x_requestcondition);
    pthread_cond_wait(&x->x_answercondition, &x->x_mutex);
  }
  //x->x_bytespersample = bytespersamp;
  x->x_filename = filesym->s_name;
  x->x_fileformat = fileformat;
  x->x_requestcode = REQUEST_OPEN;
  x->x_sampleformat = sampleformat;

  x->x_fifotail = 0;
  x->x_fifohead = 0;
  x->x_eof = 0;
  x->x_fileerror = 0;

  x->x_state = STATE_STARTUP;

  if(0){}
#if 0
  else if (samplerate > 0)
    x->x_samplerate = samplerate;
#endif
  else if (x->x_insamplerate > 0)
    x->x_samplerate = x->x_insamplerate;
  else x->x_samplerate = sys_getsr();

  /* set fifosize from bufsize.  fifosize must be a
     multiple of the number of bytes eaten for each DSP
     tick.  */

  x->x_fifosize = x->x_bufframes-(x->x_bufframes%x->x_vecsize);
  /* arrange for the "request" condition to be signalled 16
     times per buffer */
  x->x_sigcountdown = x->x_sigperiod = (x->x_fifosize / (16 * x->x_vecsize));

  pthread_cond_signal(&x->x_requestcondition);
  pthread_mutex_unlock(&x->x_mutex);
}

static void ambix_write_dsp(t_ambix_write *x, t_signal **sp) {
  int i, ninlets = x->x_ambichannels+x->x_extrachannels;
  pthread_mutex_lock(&x->x_mutex);
  x->x_vecsize = sp[0]->s_n;

  x->x_fifosize = x->x_bufframes-(x->x_bufframes%x->x_vecsize);
  x->x_sigperiod = (x->x_fifosize / (16 * x->x_vecsize));

  for (i = 0; i < ninlets; i++)
    x->x_invec[i] = sp[i]->s_vec;
  x->x_insamplerate = sp[0]->s_sr;
  pthread_mutex_unlock(&x->x_mutex);
  dsp_add(ambix_write_perform, 1, x);
}

static void ambix_write_print(t_ambix_write *x) {
  post("state %d", x->x_state);
  post("fifo head %d", x->x_fifohead);
  post("fifo tail %d", x->x_fifotail);
  post("fifo size %d", x->x_fifosize);
  post("eof %d", x->x_eof);
}

static void ambix_write_free(t_ambix_write *x) {
  /* request QUIT and wait for acknowledge */
  void *threadrtn;
  pthread_mutex_lock(&x->x_mutex);
  x->x_requestcode = REQUEST_QUIT;
  /* post("stopping ambix_write thread..."); */
  pthread_cond_signal(&x->x_requestcondition);
  while (x->x_requestcode != REQUEST_NOTHING) {
    /* post("signalling..."); */
    pthread_cond_signal(&x->x_requestcondition);
    pthread_cond_wait(&x->x_answercondition, &x->x_mutex);
  }
  pthread_mutex_unlock(&x->x_mutex);
  if (pthread_join(x->x_childthread, &threadrtn))
    pd_error(x, "ambix_write_free: join failed");
  /* post("... done."); */

  pthread_cond_destroy(&x->x_requestcondition);
  pthread_cond_destroy(&x->x_answercondition);
  pthread_mutex_destroy(&x->x_mutex);
  freebytes(x->x_buf, x->x_bufsize);

  if(x->x_matrix)
    ambix_matrix_destroy(x->x_matrix);
  x->x_matrix=NULL;
}
static void printmatrix(const ambix_matrix_t*mtx) {
  if(mtx) {
    float32_t**data=mtx->data;
    uint32_t r, c;
    post(" [%dx%d] = %p", mtx->rows, mtx->cols, mtx->data);
    for(r=0; r<mtx->rows; r++) {
      for(c=0; c<mtx->cols; c++) {
        startpost("%08f ", data[r][c]);
      }
      endpost();
    }
  }
  endpost();
}

static void ambix_write_matrix(t_ambix_write *x, t_symbol*s, int argc, t_atom*argv) {
  int rows, cols;
  float32_t*data=NULL;
  int count;
  if(x->x_matrix)
    ambix_matrix_destroy(x->x_matrix);
  x->x_matrix=NULL;
  if(argc>=2) {
    rows=atom_getint(argv+0);
    cols=atom_getint(argv+1);
    argc-=2;
    argv+=2;
  } else {
    pd_error(x, "invalid matrix message");
    return;
  }
  if(argc!=rows*cols) {
    pd_error(x, "invalid matrix");
    return;
  }
  data=(float32_t*)malloc(rows*cols*sizeof(float32_t));
  for(count=0; count<argc; count++) {
    data[count]=atom_getfloat(argv+count);
  }

  x->x_matrix=ambix_matrix_init(rows, cols, x->x_matrix);
  if(AMBIX_ERR_SUCCESS!=ambix_matrix_fill_data(x->x_matrix, data)) {
    pd_error(x, "invalid matrix data [%dx%d]=%p", rows, cols, data);
    if(x->x_matrix)
      ambix_matrix_destroy(x->x_matrix);
    x->x_matrix=NULL;
  }
  free(data);

  if(x->x_matrix)
    printmatrix(x->x_matrix);
}

AMBIX_EXPORT
void ambix_write_tilde_setup(void) {
  ambix_write_class = class_new(gensym("ambix_write~"), (t_newmethod)ambix_write_new,
                            (t_method)ambix_write_free, sizeof(t_ambix_write), 0, A_GIMME, A_NULL);
  class_addmethod(ambix_write_class, (t_method)ambix_write_start, gensym("start"), A_NULL);
  class_addmethod(ambix_write_class, (t_method)ambix_write_stop, gensym("stop"), A_NULL);
  class_addmethod(ambix_write_class, (t_method)ambix_write_matrix, gensym("matrix"), A_GIMME, A_NULL);

  class_addmethod(ambix_write_class, (t_method)ambix_write_dsp, gensym("dsp"), A_NULL);
  class_addmethod(ambix_write_class, (t_method)ambix_write_open, gensym("open"), A_GIMME, A_NULL);
  class_addmethod(ambix_write_class, (t_method)ambix_write_print, gensym("print"), A_NULL);
  CLASS_MAINSIGNALIN(ambix_write_class, t_ambix_write, x_f);
}
