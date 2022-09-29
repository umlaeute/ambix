/* ambix_read~ -  read AMBIsonics eXchange files in Pd        -*- c -*-

 Copyright © 1997-1999, Miller Puckette <msp@ucsd.edu>.
 Copyright © 2012-2014, IOhannes m zmölnig <zmoelnig@iem.at>.
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

 [ambix_read~] is based on Pd's [ambix_read~], which is released under a
 3-clause BSD-License ("Standard Improved BSD License")

*/

#if 0
# define MARK printf("%s[%d]:%s\t", __FILE__, __LINE__, __FUNCTION__), printf
#else
static void noop(const char*format, ...) {}
# define MARK noop
#endif
#ifndef _WIN32
# include <unistd.h>
# include <fcntl.h>
#endif
#include <pthread.h>
#ifdef _WIN32
# include <io.h>
#endif

#include <stdlib.h>
#include <stdio.h>
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

/* merge to buffers of interleaved samples into a single interleaved buffer
 * buf1 holds chan1 samples per frame, buf2 holds chan2 samples per frame
 * the dest buffer holds (want1+want2) samples per frame
 * if chan<want, samples are dropped, if want>chan ZERO samples are filled in
 */
static void merge_samples(float32_t*buf1, uint32_t chan1, uint32_t want1,
                          float32_t*buf2, uint32_t chan2, uint32_t want2,
                          t_sample*dest, uint32_t destsize,
                          uint32_t offset, uint32_t frames) {
  uint32_t left1=(want1>chan1)?want1-chan1:0;
  uint32_t left2=(want2>chan2)?want2-chan2:0;
  uint32_t use1=(chan1<want1)?chan1:want1;
  uint32_t use2=(chan2<want2)?chan2:want2;

  const uint32_t framesize=want1+want2;

  uint32_t f, c;
  for(f=0; f<frames; f++) {
    float32_t*in=NULL;
    t_sample*out=dest+((offset+f)%destsize)*framesize;
    /* copy samples from buf1 */
    in=buf1+(f*chan1);
    for(c=0; c<use1; c++)
      *out++=*in++;
    /* zero-pad if needed */
    for(c=0; c<left1; c++)
      *out++=0.;
    /* copy samples from buf2 */
    in=buf2+(f*chan2);
    for(c=0; c<use2; c++)
      *out++=*in++;
    /* zero-pad if needed */
    for(c=0; c<left2; c++)
      *out++=0.;
  }
}

/* takes a buffer of interleaved samples (channels samples per frame), and splits it
 * into non-interleaved Pd-channels */
static void deinterleave_samples(t_sample*inbuf, uint32_t channels,
                                 t_sample**outvecs, uint32_t frames) {
  uint32_t f, c;
  for(f=0; f<frames; f++) {
    for(c=0; c<channels; c++) {
      outvecs[c][f]=*inbuf++;
    }
  }
}



/* tries to expand the given filename to it's full glory) */
t_symbol*get_filename(t_canvas*canvas, t_symbol*s) {
  int fd=0;
  char buf[MAXPDSTRING];
  char result[MAXPDSTRING];
  char*bufptr;

  if(!s || !s->s_name || !*s->s_name) {
    return NULL;
  }

  if ((fd=canvas_open(canvas, s->s_name, "", buf, &bufptr, MAXPDSTRING, 1))>=0){
    sys_close(fd);
    snprintf(result, MAXPDSTRING-1, "%s/%s", buf, bufptr);
    result[MAXPDSTRING-1]=0;
    return gensym(result);
  } else if(canvas) {
    canvas_makefilename(canvas, s->s_name, result, MAXPDSTRING);
    return gensym(result);
  }
  return s;
}


/************************* ambix_read object ******************************/

/* [ambix_read~] uses the Posix threads package; for the moment we're Linux
   only although this should be portable to the other platforms.

   Each instance of ambix_read~ owns a "child" thread for doing the unix (MSW?) file
   reading.  The parent thread signals the child each time:
   (1) a file wants opening or closing;
   (2) we've eaten another 1/16 of the shared buffer (so that the
   child thread should check if it's time to read some more.)
   The child signals the parent whenever a read has completed.  Signalling
   is done by setting "conditions" and putting data in mutex-controlled common
   areas.
*/

static t_class *ambix_read_class;

typedef struct _infoflags {
  int f_eof;     /* EOF notification */
  int f_matrix;  /* send matrix */
  int f_ambix;   /* send ambix_info */
} t_infoflags;

typedef struct _ambix_read {
  t_object x_obj;
  t_canvas *x_canvas;
  t_sample *x_buf;                        /* soundfile buffer */
  int x_bufsize;                          /* buffer size in bytes */
  int x_bufframes;                        /* buffer size in frames */
  int x_noutlets;                         /* number of audio outlets */
  t_sample *(x_outvec[MAXSFCHANS]);       /* audio vectors */

  t_clock *x_clock;                       /* to call back on EOF */
  t_outlet *x_infoout;                    /* bang-on-done outlet */
  t_outlet *x_matrixout;                  /* bang-on-done outlet */
  t_infoflags x_infoflags;                /* what to do in the callback */

  uint32_t x_ambichannels;                /* ambichannels of the object (const) */
  uint32_t x_xtrachannels;                /* xtrachannels of the object (const) */
  ambix_fileformat_t x_fileformat;        /* readmode of the object (const) */

  int x_vecsize;                          /* vector size for transfers */

  int x_state;                            /* opened, running, or idle */
  int x_requestcode;                      /* pending request from parent to I/O thread */

  t_float x_insamplerate;   /* sample rate of input signal if known */

  /* parameters to communicate with subthread */
  const char *x_filename;       /* file to open (string is permanently allocated) */
  int x_fileerror;        /* slot for "errno" return */
  t_float x_samplerate;     /* sample rate of soundfile */
  long x_onsetframes;     /* number of sample frames to skip */

  ambix_matrix_t x_matrix;
  ambix_info_t   x_ambix;
  ambix_t        *x_ambix_t;

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
} t_ambix_read;


/************** the child thread which performs file I/O ***********/

static void *ambix_read_child_main(void *zz) {
  t_ambix_read *x = (t_ambix_read*)zz;
  x->x_ambix_t=NULL;
  const uint32_t want_ambichannels    = x->x_ambichannels;
  const uint32_t want_xtrachannels    = x->x_xtrachannels;
  const ambix_fileformat_t want_fileformat = x->x_fileformat;
  pthread_mutex_lock(&x->x_mutex);

  while (1) {
    int fifohead;
    t_sample *buf=NULL;
    if (x->x_requestcode == REQUEST_NOTHING) {
      pthread_cond_signal(&x->x_answercondition);
      pthread_cond_wait(&x->x_requestcondition, &x->x_mutex);
    } else if (x->x_requestcode == REQUEST_OPEN) {
      ambix_info_t ainfo;
      int64_t sysrtn;
      int wantframes;
      const ambix_matrix_t*matrix=NULL;

      /* copy file stuff out of the data structure so we can
         relinquish the mutex while we're in open_soundfile(). */
      long onsetframes = x->x_onsetframes;

      int localfifosize = x->x_fifosize;
      uint32_t ambichannels=0, xtrachannels=0; /* get this from ainfo */

      float32_t*ambibuf = NULL;
      float32_t*xtrabuf = NULL;

      char *filename = strndup(x->x_filename, MAXPDSTRING);
      //      char *dirname = canvas_getdir(x->x_canvas)->s_name;
      /* alter the request code so that an ensuing "open" will get
         noticed. */
      x->x_requestcode = REQUEST_BUSY;
      x->x_fileerror = 0;

      /* open the soundfile with the mutex unlocked */
      pthread_mutex_unlock(&x->x_mutex);

      memset(&ainfo, 0, sizeof(ainfo));
      ainfo.fileformat=want_fileformat;
      if (x->x_ambix_t)
        ambix_close(x->x_ambix_t);
      x->x_ambix_t=ambix_open(filename, AMBIX_READ, &ainfo);
      free(filename);

      if(x->x_ambix_t) {
        matrix=ambix_get_adaptormatrix(x->x_ambix_t);
        if(onsetframes) {
          ambix_seek(x->x_ambix_t, onsetframes, SEEK_SET);
        }
      }

      ambichannels=ainfo.ambichannels;
      xtrachannels=ainfo.extrachannels;

      ambibuf = (float32_t*)calloc(localfifosize*ambichannels, sizeof(float32_t));
      xtrabuf = (float32_t*)calloc(localfifosize*xtrachannels, sizeof(float32_t));

      pthread_mutex_lock(&x->x_mutex);

      if (NULL==x->x_ambix_t) {
        x->x_fileerror = errno;
        x->x_eof = 1;
        goto lost;
      }

      /* check if another request has been made; if so, field it */
      if (x->x_requestcode != REQUEST_BUSY)
        goto lost;

      /* copy back into the instance structure. */
      if(matrix) {
        ambix_matrix_copy(matrix, &x->x_matrix);
        x->x_infoflags.f_matrix=1;
      }
      memcpy(&x->x_ambix, &ainfo, sizeof(ainfo));
      x->x_infoflags.f_ambix=1;

      /* clear the FIFO */
      memset(x->x_buf, 0, x->x_bufsize);

      x->x_fifohead = 0;
      /* set fifosize from bufsize.  fifosize must be a
         multiple of the number of bytes eaten for each DSP
         tick.  We pessimistically assume MAXVECSIZE samples
         per tick since that could change.  There could be a
         problem here if the vector size increases while a
         soundfile is being played...  */
      x->x_fifosize = x->x_bufframes-(x->x_bufframes%x->x_vecsize);
      /* arrange for the "request" condition to be signalled 16
         times per buffer */
      x->x_sigcountdown = x->x_sigperiod = (x->x_fifosize / (16 * x->x_vecsize));
      /* in a loop, wait for the fifo to get hungry and feed it */

      while (x->x_requestcode == REQUEST_BUSY) {
        int fifosize = x->x_fifosize;
        int bufframes = 0;
        if (x->x_eof)
          break;
        if (x->x_fifohead >= x->x_fifotail) {
          /* if the head is >= the tail, we can immediately read
             to the end of the fifo.  Unless, that is, we would
             read all the way to the end of the buffer and the
             "tail" is zero; this would fill the buffer completely
             which isn't allowed because you can't tell a completely
             full buffer from an empty one. */
          if (x->x_fifotail || (fifosize - x->x_fifohead > READFRAMES)) {
            wantframes = fifosize - x->x_fifohead;
            if (wantframes > READFRAMES)
              wantframes = READFRAMES;
          } else {
            pthread_cond_signal(&x->x_answercondition);
            pthread_cond_wait(&x->x_requestcondition, &x->x_mutex);
            continue;
          }
        } else {
          /* otherwise check if there are at least READFRAMES
             bytes to read.  If not, wait and loop back. */
          wantframes =  x->x_fifotail - x->x_fifohead - 1;
          if (wantframes < READFRAMES) {
            pthread_cond_signal(&x->x_answercondition);
            pthread_cond_wait(&x->x_requestcondition, &x->x_mutex);
            continue;
          } else
            wantframes = READFRAMES;
        }
        buf = x->x_buf;
        fifohead = x->x_fifohead;

        bufframes = x->x_bufframes;

        pthread_mutex_unlock(&x->x_mutex);

        if(localfifosize<fifosize) {
          free(ambibuf); free(xtrabuf);
          localfifosize=fifosize;
          ambibuf = (float32_t*)calloc(localfifosize*ambichannels, sizeof(float32_t));
          xtrabuf = (float32_t*)calloc(localfifosize*xtrachannels, sizeof(float32_t));
        }
        sysrtn = ambix_readf_float32(x->x_ambix_t, ambibuf, xtrabuf, wantframes);
        if(sysrtn>0) {
          merge_samples(ambibuf, ambichannels, want_ambichannels,
                        xtrabuf, xtrachannels, want_xtrachannels,
                        buf, bufframes,
                        fifohead, sysrtn);
        }
        pthread_mutex_lock(&x->x_mutex);
        if (x->x_requestcode != REQUEST_BUSY)
          break;
        if (sysrtn < 0) {
          x->x_fileerror = errno;
          break;
        } else if (sysrtn == 0) {
          x->x_eof = 1;
          break;
        } else {
          x->x_fifohead += sysrtn;
          if (x->x_fifohead == fifosize)
            x->x_fifohead = 0;
        }
        /* signal parent in case it's waiting for data */
        pthread_cond_signal(&x->x_answercondition);
      }
    lost:
      free(ambibuf); free(xtrabuf);

      if (x->x_requestcode == REQUEST_BUSY)
        x->x_requestcode = REQUEST_NOTHING;
      /* fell out of read loop: close file if necessary,
         set EOF and signal once more */
      if(x->x_ambix_t) {
        pthread_mutex_unlock(&x->x_mutex);
        ambix_close(x->x_ambix_t);
        x->x_ambix_t=NULL;
        pthread_mutex_lock(&x->x_mutex);
      }

      pthread_cond_signal(&x->x_answercondition);

    } else if (x->x_requestcode == REQUEST_CLOSE) {
      if(x->x_ambix_t) {
        pthread_mutex_unlock(&x->x_mutex);
        ambix_close(x->x_ambix_t);
        x->x_ambix_t=NULL;
        pthread_mutex_lock(&x->x_mutex);
      }
      if (x->x_requestcode == REQUEST_CLOSE)
        x->x_requestcode = REQUEST_NOTHING;
      pthread_cond_signal(&x->x_answercondition);
    } else if (x->x_requestcode == REQUEST_QUIT) {
      if(x->x_ambix_t) {
        pthread_mutex_unlock(&x->x_mutex);
        ambix_close(x->x_ambix_t);
        x->x_ambix_t=NULL;
        pthread_mutex_lock(&x->x_mutex);
      }
      x->x_requestcode = REQUEST_NOTHING;
      pthread_cond_signal(&x->x_answercondition);
      break;
    } else {
    }
  }
  pthread_mutex_unlock(&x->x_mutex);
  return (0);
}

/******** the object proper runs in the calling (parent) thread ****/

static void ambix_read_tick(t_ambix_read *x);

static void *ambix_read_new(t_symbol*s, int argc, t_atom*argv) {
  int achannels=0, xchannels=0, bufframes=-1, bufsize=0;
  t_ambix_read *x;
  int nchannels, i;
  t_sample *buf=NULL;
  int have_x=0, limiting=0;

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
    bufframes=atom_getint(argv+2);
    break;
  default:
    pd_error(0, "usage: [%s~ <ambichannels> <extrachannels> <buffersize>]", s->s_name);
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

  if(limiting) {
    /* FIXXME: warning on channel limiting */
  }
  if(have_x) {
    /* FIXXME: remove 'have_x' variable */
  }

  if (bufframes <= 0) bufframes = DEFBUFPERCHAN;
  else if (bufframes < MINBUFSIZE)
    bufframes = MINBUFSIZE;
  else if (bufframes > MAXBUFSIZE)
    bufframes = MAXBUFSIZE;
  bufsize=bufframes*nchannels;
  buf = (t_sample*)getbytes(bufsize*sizeof(t_sample));
  if (!buf) return (0);

  x = (t_ambix_read *)pd_new(ambix_read_class);

  x->x_fileformat=(gensym("ambix_read~")==s)?AMBIX_BASIC:AMBIX_EXTENDED;
  memset(&x->x_matrix, 0, sizeof(x->x_matrix));


  if(AMBIX_EXTENDED==x->x_fileformat)
    x->x_matrixout = outlet_new(&x->x_obj, 0);
  else
    x->x_matrixout = NULL;

  for (i = 0; i < nchannels; i++)
    outlet_new(&x->x_obj, gensym("signal"));
  x->x_noutlets = nchannels;
  x->x_infoout = outlet_new(&x->x_obj, &s_bang);

  x->x_ambichannels = achannels;
  x->x_xtrachannels = xchannels;

  x->x_canvas = canvas_getcurrent();

  pthread_mutex_init(&x->x_mutex, 0);
  pthread_cond_init(&x->x_requestcondition, 0);
  pthread_cond_init(&x->x_answercondition, 0);

  pthread_mutex_lock(&x->x_mutex);
  x->x_vecsize = DEFAULTVECSIZE;
  x->x_state = STATE_IDLE;
  x->x_clock = clock_new(x, (t_method)ambix_read_tick);
  x->x_buf = buf;
  x->x_bufsize = bufsize;
  x->x_bufframes = bufframes;
  x->x_fifosize = x->x_fifohead = x->x_fifotail = x->x_requestcode = 0;
  pthread_mutex_unlock(&x->x_mutex);

  pthread_create(&x->x_childthread, 0, ambix_read_child_main, x);
  return (x);
}

static void ambix_read_tick(t_ambix_read *x) {
  pthread_mutex_lock(&x->x_mutex);

  if(x->x_infoflags.f_eof)
    outlet_bang(x->x_infoout);

  if(x->x_infoflags.f_ambix) {
    t_atom atoms[1];

    /* number of ambisonics channels */
    SETFLOAT(atoms+0, (t_float)(x->x_ambix.ambichannels));
    outlet_anything(x->x_infoout, gensym("ambichannels"), 1, atoms);

    /* number of non-ambisonics channels */
    SETFLOAT(atoms+0, (t_float)(x->x_ambix.extrachannels));
    outlet_anything(x->x_infoout, gensym("extrachannels"), 1, atoms);

    /* playback samplerate (might be different if we did resampling) */
    SETFLOAT(atoms+0, (t_float)(x->x_ambix.samplerate));
    outlet_anything(x->x_infoout, gensym("resamplerate"), 1, atoms);

    /* samplerate of file */
    SETFLOAT(atoms+0, (t_float)(x->x_ambix.samplerate));
    outlet_anything(x->x_infoout, gensym("samplerate"), 1, atoms);

    /* number of sample frames in file */
    SETFLOAT(atoms+0, (t_float)(x->x_ambix.frames));
    outlet_anything(x->x_infoout, gensym("frames"), 1, atoms);

    /* number of markers in the file */
    SETFLOAT(atoms+0, (t_float)(ambix_get_num_markers(x->x_ambix_t)));
    outlet_anything(x->x_infoout, gensym("num_markers"), 1, atoms);

    /* number of regions in the file */
    SETFLOAT(atoms+0, (t_float)(ambix_get_num_regions(x->x_ambix_t)));
    outlet_anything(x->x_infoout, gensym("num_regions"), 1, atoms);
  }



  if(x->x_infoflags.f_matrix && x->x_matrixout) {
    int size=x->x_matrix.rows*x->x_matrix.cols;
    if(size) {
      uint32_t r, c, index;
      t_atom*ap=(t_atom*)getbytes(sizeof(t_atom)*(size+2));
      SETFLOAT(ap+0, x->x_matrix.rows);
      SETFLOAT(ap+1, x->x_matrix.cols);

      index=2;
      for(r=0; r<x->x_matrix.rows; r++) {
        for(c=0; c<x->x_matrix.cols; c++) {
          SETFLOAT(ap+index, x->x_matrix.data[r][c]);
          index++;
        }
      }

      outlet_anything(x->x_matrixout, gensym("matrix"), size+2, ap);
      freebytes(ap, sizeof(t_atom)*(size+2));
    }
  }

  memset(&x->x_infoflags, 0, sizeof(x->x_infoflags));

  pthread_mutex_unlock(&x->x_mutex);
}

static t_int *ambix_read_perform(t_int *w) {
  t_ambix_read *x = (t_ambix_read *)(w[1]);
  int vecsize = x->x_vecsize, noutlets = x->x_noutlets, i, j;
  t_sample *fp;
  int skip=0;

  if(x->x_infoflags.f_matrix || x->x_infoflags.f_ambix ) {
    clock_delay(x->x_clock, 0);
    skip=1;
  }

  if (!skip && x->x_state == STATE_STREAM) {
    int wantframes;
    pthread_mutex_lock(&x->x_mutex);
    wantframes = vecsize;

    while (
           !x->x_eof &&
           x->x_fifohead >= x->x_fifotail &&
           x->x_fifohead < x->x_fifotail + wantframes-1
           ) {
      pthread_cond_signal(&x->x_requestcondition);
      pthread_cond_wait(&x->x_answercondition, &x->x_mutex);
      /* resync local variables -- bug fix thanks to Shahrokh */
      vecsize = x->x_vecsize;
      wantframes = vecsize;
    }

    /* check for EOF (and buffer is about to drain) */
    if (x->x_eof &&
        x->x_fifohead >= x->x_fifotail &&
        x->x_fifohead <  x->x_fifotail + wantframes-1
        ) {
      int xfersize;
      if (x->x_fileerror) {
        pd_error(x, "dsp: %s: %s", x->x_filename,
                 (x->x_fileerror == EIO ?
                  "unknown or bad header format" :
                  strerror(x->x_fileerror)));
      }
      x->x_infoflags.f_eof=1;
      clock_delay(x->x_clock, 0);
      x->x_state = STATE_IDLE;

      /* if there's a partial buffer left, copy it out. */
      xfersize = (x->x_fifohead - x->x_fifotail + 1);
      if (xfersize) {
        deinterleave_samples(x->x_buf+(x->x_fifotail*noutlets),
                             noutlets,
                             x->x_outvec,
                             xfersize);
        vecsize -= xfersize;
      }
      /* then zero out the (rest of the) output */
      for (i = 0; i < noutlets; i++)
        for (j = vecsize, fp = x->x_outvec[i] + xfersize; j--; )
          *fp++ = 0;

      pthread_cond_signal(&x->x_requestcondition);
      pthread_mutex_unlock(&x->x_mutex);
      return (w+2);
    }


    deinterleave_samples(x->x_buf+(x->x_fifotail*noutlets),
                         noutlets,
                         x->x_outvec,
                         vecsize);
    x->x_fifotail += wantframes;
    if (x->x_fifotail >= x->x_fifosize) {
      x->x_fifotail = 0;
    }
    if ((--x->x_sigcountdown) <= 0) {
      pthread_cond_signal(&x->x_requestcondition);
      x->x_sigcountdown = x->x_sigperiod;
    }
    pthread_mutex_unlock(&x->x_mutex);
  } else {
    for (i = 0; i < noutlets; i++)
      for (j = vecsize, fp = x->x_outvec[i]; j--; )
        *fp++ = 0;
  }
  return (w+2);
}

static void ambix_read_start(t_ambix_read *x) {
  /* start making output.  If we're in the "startup" state change
     to the "running" state. */
  pthread_mutex_lock(&x->x_mutex);
  if (x->x_state == STATE_STARTUP) {
    x->x_state = STATE_STREAM;
    pthread_mutex_unlock(&x->x_mutex);
  } else {
    pthread_mutex_unlock(&x->x_mutex);
    pd_error(x, "ambix_read~: start requested with no prior 'open'");
  }
}

static void ambix_read_stop(t_ambix_read *x) {
  /* LATER rethink whether you need the mutex just to set a variable? */
  pthread_mutex_lock(&x->x_mutex);
  x->x_state = STATE_IDLE;
  x->x_requestcode = REQUEST_CLOSE;
  pthread_cond_signal(&x->x_requestcondition);
  pthread_mutex_unlock(&x->x_mutex);
}

static void ambix_read_float(t_ambix_read *x, t_floatarg f) {
  if (f != 0)
    ambix_read_start(x);
  else ambix_read_stop(x);
}

/* open method.  Called as:
   open filename [skipframes]
   (if headersize is zero, header is taken to be automatically
   detected; thus, use the special "-1" to mean a truly headerless file.)
*/

static void ambix_read_open(t_ambix_read*x, t_symbol*s, t_float onsetframes) {
  t_symbol*filesym=get_filename(x->x_canvas, s);
  if (!filesym)
    return;

  pthread_mutex_lock(&x->x_mutex);
  x->x_requestcode = REQUEST_OPEN;
  x->x_filename = filesym->s_name;
  x->x_fifotail = 0;
  x->x_fifohead = 0;
  x->x_onsetframes = (onsetframes > 0 ? onsetframes : 0);
  x->x_eof = 0;
  x->x_fileerror = 0;
  x->x_state = STATE_STARTUP;
  pthread_cond_signal(&x->x_requestcondition);
  pthread_mutex_unlock(&x->x_mutex);
}

static void ambix_read_dsp(t_ambix_read *x, t_signal **sp) {
  int i, noutlets = x->x_noutlets;
  pthread_mutex_lock(&x->x_mutex);
  x->x_vecsize = sp[0]->s_n;

  x->x_sigperiod = (x->x_fifosize / (16 * x->x_vecsize));

  for (i = 0; i < noutlets; i++)
    x->x_outvec[i] = sp[i]->s_vec;
  pthread_mutex_unlock(&x->x_mutex);
  dsp_add(ambix_read_perform, 1, x);
}

static void ambix_read_print(t_ambix_read *x) {
  post("state %d", x->x_state);
  post("fifo head %d", x->x_fifohead);
  post("fifo tail %d", x->x_fifotail);
  post("fifo size %d", x->x_fifosize);
  post("eof %d", x->x_eof);

#if 0
  if(1) {
    int c, f=0;
    int frames=x->x_fifosize;
    int channels=x->x_noutlets;
    float32_t*buf=x->x_buf;
    for(f=0; f<x->x_fifosize; f++) {
      startpost("frame[%d]:", f);
      for(c=0; c<channels; c++) {
        startpost(" %g", *buf++);
      }
      endpost();
    }
  }
#endif
}

static void ambix_read_free(t_ambix_read *x) {
  /* request QUIT and wait for acknowledge */
  void *threadrtn;
  pthread_mutex_lock(&x->x_mutex);
  x->x_requestcode = REQUEST_QUIT;
  pthread_cond_signal(&x->x_requestcondition);
  while (x->x_requestcode != REQUEST_NOTHING) {
    pthread_cond_signal(&x->x_requestcondition);
    pthread_cond_wait(&x->x_answercondition, &x->x_mutex);
  }
  pthread_mutex_unlock(&x->x_mutex);
  if (pthread_join(x->x_childthread, &threadrtn))
    pd_error(x, "ambix_read_free: join failed");

  pthread_cond_destroy(&x->x_requestcondition);
  pthread_cond_destroy(&x->x_answercondition);
  pthread_mutex_destroy(&x->x_mutex);
  freebytes(x->x_buf, x->x_bufsize);
  clock_free(x->x_clock);

  outlet_free(x->x_infoout);
  if(x->x_matrixout)
    outlet_free(x->x_matrixout);

  ambix_matrix_deinit(&x->x_matrix);
}

static void ambix_read_marker(t_ambix_read*x, t_float marker_id) {
  if (!x->x_ambix_t)
    return;
  if ( ((int)marker_id >= 0) && ((int)marker_id < ambix_get_num_markers(x->x_ambix_t)) ) {
    ambix_marker_t *marker;
    marker = ambix_get_marker(x->x_ambix_t, (int)marker_id);
    if (marker) {
      t_atom atoms[3]; // id pos name
      SETFLOAT(atoms+0, (t_float)(int)marker_id);
      SETFLOAT(atoms+1, (t_float)marker->position);
      SETSYMBOL(atoms+2, gensym(marker->name));
      outlet_anything(x->x_infoout, gensym("marker"), 3, atoms);
    }
  } else {
    pd_error(x, "ambix_read~: no marker with this id in file");
  }
}

static void ambix_read_all_markers(t_ambix_read*x) {
  int nummarkers, i;
  if (!x->x_ambix_t)
    return;
  nummarkers = ambix_get_num_markers(x->x_ambix_t);
  for (i=0; i<nummarkers; i++) {
    ambix_read_marker(x, i);
  }
}

static void ambix_read_region(t_ambix_read*x, t_float region_id) {
  if (!x->x_ambix_t)
    return;
  if ( ((int)region_id >= 0) && ((int)region_id < ambix_get_num_regions(x->x_ambix_t)) ) {
    ambix_region_t *region;
    region = ambix_get_region(x->x_ambix_t, (int)region_id);
    if (region) {
      t_atom atoms[4]; // id start_pos end_pos name
      SETFLOAT(atoms+0, (t_float)(int)region_id);
      SETFLOAT(atoms+1, (t_float)region->start_position);
      SETFLOAT(atoms+2, (t_float)region->end_position);
      SETSYMBOL(atoms+3, gensym(region->name));
      outlet_anything(x->x_infoout, gensym("region"), 4, atoms);
    }
  } else {
    pd_error(x, "ambix_read~: no region with this id in file");
  }
}

static void ambix_read_all_regions(t_ambix_read*x) {
  int numregions, i;
  if (!x->x_ambix_t)
    return;
  numregions = ambix_get_num_regions(x->x_ambix_t);
  for (i=0; i<numregions; i++) {
    ambix_read_region(x, i);
  }
}

static void ambix_seek_pos(t_ambix_read*x, t_float position) {
  if (!x->x_ambix_t) {
    pd_error(x, "ambix_read~: seek not possible, requested with no prior 'open'");
    return;
  }
  pthread_mutex_lock(&x->x_mutex);
  if (x->x_state == STATE_STARTUP) {
    int64_t ret = ambix_seek(x->x_ambix_t, (int64_t)position, SEEK_SET);
    if (ret < 0)
      pd_error(x, "ambix_read~: seek not possible");
    pthread_mutex_unlock(&x->x_mutex);
  } else {
    pthread_mutex_unlock(&x->x_mutex);
    pd_error(x, "ambix_read~: seek not possible, playback already started");
  }
}

AMBIX_EXPORT
void ambix_read_tilde_setup(void) {
  ambix_read_class = class_new(gensym("ambix_read~"), (t_newmethod)ambix_read_new,
                               (t_method)ambix_read_free, sizeof(t_ambix_read), 0, A_GIMME, A_NULL);
  class_addcreator((t_newmethod)ambix_read_new, gensym("ambix_readX~"), A_GIMME, 0);

  class_addfloat(ambix_read_class, (t_method)ambix_read_float);
  class_addmethod(ambix_read_class, (t_method)ambix_read_start, gensym("start"), A_NULL);
  class_addmethod(ambix_read_class, (t_method)ambix_read_stop, gensym("stop"), A_NULL);
  class_addmethod(ambix_read_class, (t_method)ambix_read_dsp, gensym("dsp"), A_NULL);
  class_addmethod(ambix_read_class, (t_method)ambix_read_open, gensym("open"), A_SYMBOL, A_DEFFLOAT, A_NULL);
  class_addmethod(ambix_read_class, (t_method)ambix_read_print, gensym("print"), A_NULL);
  class_addmethod(ambix_read_class, (t_method)ambix_read_marker, gensym("get_marker"), A_DEFFLOAT, A_NULL);
  class_addmethod(ambix_read_class, (t_method)ambix_read_all_markers, gensym("get_all_markers"), A_NULL);
  class_addmethod(ambix_read_class, (t_method)ambix_read_region, gensym("get_region"), A_DEFFLOAT, A_NULL);
  class_addmethod(ambix_read_class, (t_method)ambix_read_all_regions, gensym("get_all_regions"), A_NULL);
  class_addmethod(ambix_read_class, (t_method)ambix_seek_pos, gensym("seek"), A_DEFFLOAT, A_NULL);

  if(0)
    MARK("[ambix_read~] setup done");
}

AMBIX_EXPORT
void ambix_readX_tilde_setup(void) {
  ambix_read_tilde_setup();
}
