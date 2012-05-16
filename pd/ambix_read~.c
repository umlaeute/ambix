/* ambix_read~ -  read AMBIsonics Xchange files in Pd        -*- c -*-

 Copyright © 1997-1999 Miller Puckette.
 Copyright © 2012 IOhannes m zmölnig <zmoelnig@iem.at>.
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


#ifndef _WIN32
#include <unistd.h>
#include <fcntl.h>
#endif
#include <pthread.h>
#ifdef _WIN32
#include <io.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <m_pd.h>
#include <ambix/ambix.h>

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


#if 0
#define MAXBYTESPERSAMPLE 4
#define MAXVECSIZE 128

#define READSIZE 65536
#define WRITESIZE 65536
#define DEFBUFPERCHAN 262144
#define MINBUFSIZE (4 * READSIZE)
#define MAXBUFSIZE 16777216     /* arbitrary; just don't want to hang malloc */
#endif

                               static void soundfile_xferin_sample(int sfchannels, int nvecs, t_sample **vecs,
                                                                   long itemsread, unsigned char *buf, int nitems, int bytespersamp,
                                                                   int bigendian, int spread) {
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

typedef struct _ambix_read {
  t_object x_obj;
  t_canvas *x_canvas;
  float32_t *x_buf;                       /* soundfile buffer */
  int x_bufsize;                          /* buffer size in bytes */
  int x_bufframes;                        /* buffer size in frames */
  int x_noutlets;                         /* number of audio outlets */
  t_sample *(x_outvec[MAXSFCHANS]);       /* audio vectors */
  int x_vecsize;                          /* vector size for transfers */

  t_clock *x_clock;
  t_outlet *x_bangout;                    /* bang-on-done outlet */

  int x_state;                            /* opened, running, or idle */
  t_float x_insamplerate;   /* sample rate of input signal if known */

  /* parameters to communicate with subthread */
  int x_requestcode;      /* pending request from parent to I/O thread */
  char *x_filename;       /* file to open (string is permanently allocated) */
  int x_fileerror;        /* slot for "errno" return */
  int x_sfchannels;       /* number of channels in soundfile */
  t_float x_samplerate;     /* sample rate of soundfile */
  long x_onsetframes;     /* number of sample frames to skip */
  long x_bytelimit;       /* max number of data bytes to read */

  uint32_t x_ambichannels;
  uint32_t x_xtrachannels;
  ambix_matrix_t x_matrix;
  ambix_fileformat_t x_fileformat;

  int x_fd;               /* filedesc */

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
  t_ambix_read *x = zz;
  ambix_t*ambix=NULL;

  pthread_mutex_lock(&x->x_mutex);
  while (1) {
    int fd, fifohead;
    char *buf;
    if (x->x_requestcode == REQUEST_NOTHING) {
      pthread_cond_signal(&x->x_answercondition);
      pthread_cond_wait(&x->x_requestcondition, &x->x_mutex);
    } else if (x->x_requestcode == REQUEST_OPEN) {
      ambix_info_t ainfo;
      int sysrtn, wantframes;
      const ambix_matrix_t*matrix=NULL;

      /* copy file stuff out of the data structure so we can
         relinquish the mutex while we're in open_soundfile(). */
      long onsetframes = x->x_onsetframes;
      long bytelimit = 0x7fffffff;
      int sfchannels = x->x_sfchannels;

      int localfifosize = x->x_fifosize;
      uint32_t want_ambichannels = x->x_ambichannels;
      uint32_t want_xtrachannels = x->x_xtrachannels;
      uint32_t ambichannels=0, xtrachannels=0; /* get this from ainfo */

      float32_t*ambibuf = NULL;
      float32_t*xtrabuf = NULL;

      ambix_fileformat_t fileformat = x->x_fileformat;

      char *filename = strndup(x->x_filename, MAXPDSTRING);
      //      char *dirname = canvas_getdir(x->x_canvas)->s_name;
      /* alter the request code so that an ensuing "open" will get
         noticed. */
      x->x_requestcode = REQUEST_BUSY;
      x->x_fileerror = 0;

      /* open the soundfile with the mutex unlocked */
      pthread_mutex_unlock(&x->x_mutex);

      memset(&ainfo, 0, sizeof(ainfo));
      ainfo.fileformat=fileformat;

      if (ambix)
        ambix_close(ambix);
      ambix=ambix_open(filename, AMBIX_READ, &ainfo);
      free(filename);

      if(ambix) {
        matrix=ambix_get_adaptormatrix(ambix);
        if(onsetframes) {
          ambix_seek(ambix, onsetframes, SEEK_SET);
        }
      }

      ambibuf = malloc(sizeof(float32_t)*localfifosize*ambichannels);
      xtrabuf = malloc(sizeof(float32_t)*localfifosize*xtrachannels);

      pthread_mutex_lock(&x->x_mutex);
      /* copy back into the instance structure. */

      if(matrix)
        ambix_matrix_copy(matrix, &x->x_matrix);

      x->x_sfchannels = sfchannels;
      x->x_bytelimit = bytelimit;
      if (NULL==ambix) {
        x->x_fileerror = errno;
        x->x_eof = 1;
        goto lost;
      }
      /* check if another request has been made; if so, field it */
      if (x->x_requestcode != REQUEST_BUSY)
        goto lost;
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
        int fifosize = x->x_fifosize, fifotail;
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
            if (wantframes > x->x_bytelimit)
              wantframes = x->x_bytelimit;
          } else {
            pthread_cond_signal(&x->x_answercondition);
            pthread_cond_wait(&x->x_requestcondition,
                              &x->x_mutex);
            continue;
          }
        } else {
          /* otherwise check if there are at least READFRAMES
             bytes to read.  If not, wait and loop back. */
          wantframes =  x->x_fifotail - x->x_fifohead - 1;
          if (wantframes < READFRAMES) {
            pthread_cond_signal(&x->x_answercondition);
            pthread_cond_wait(&x->x_requestcondition,
                              &x->x_mutex);
            continue;
          } else
            wantframes = READFRAMES;
          if (wantframes > x->x_bytelimit)
            wantframes = x->x_bytelimit;
        }
        fd = x->x_fd;
        buf = x->x_buf;
        fifohead = x->x_fifohead;
        fifotail = x->x_fifotail;

        pthread_mutex_unlock(&x->x_mutex);

        if(localfifosize<fifosize) {
          free(ambibuf); free(xtrabuf);
          localfifosize=fifosize;
          ambibuf = malloc(sizeof(float32_t)*localfifosize*ambichannels);
          xtrabuf = malloc(sizeof(float32_t)*localfifosize*xtrachannels);
        }

        sysrtn = ambix_readf_float32(ambix, ambibuf, xtrabuf, wantframes);

        merge_samples(ambibuf, ambichannels,
                      xtrabuf, xtrachannels,
                      buf+fifotail*(ambichannels+xtrachannels), sysrtn);

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
          x->x_bytelimit -= sysrtn;
          if (x->x_bytelimit <= 0) {
            x->x_eof = 1;
            break;
          }
          if (x->x_fifohead == fifosize)
            x->x_fifohead = 0;
        }
        /* signal parent in case it's waiting for data */
        pthread_cond_signal(&x->x_answercondition);
      }
    lost:

      if (x->x_requestcode == REQUEST_BUSY)
        x->x_requestcode = REQUEST_NOTHING;
      /* fell out of read loop: close file if necessary,
         set EOF and signal once more */
      if (x->x_fd >= 0) {
        fd = x->x_fd;
        pthread_mutex_unlock(&x->x_mutex);
        close (fd);
        pthread_mutex_lock(&x->x_mutex);
        x->x_fd = -1;
      }
      pthread_cond_signal(&x->x_answercondition);

    } else if (x->x_requestcode == REQUEST_CLOSE) {
      if (x->x_fd >= 0) {
        fd = x->x_fd;
        pthread_mutex_unlock(&x->x_mutex);
        close (fd);
        pthread_mutex_lock(&x->x_mutex);
        x->x_fd = -1;
      }
      if (x->x_requestcode == REQUEST_CLOSE)
        x->x_requestcode = REQUEST_NOTHING;
      pthread_cond_signal(&x->x_answercondition);
    } else if (x->x_requestcode == REQUEST_QUIT) {
      if (x->x_fd >= 0) {
        fd = x->x_fd;
        pthread_mutex_unlock(&x->x_mutex);
        close (fd);
        pthread_mutex_lock(&x->x_mutex);
        x->x_fd = -1;
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

static void *ambix_read_new(t_floatarg fnchannels, t_floatarg fbufsize) {
  t_ambix_read *x;
  int nchannels = fnchannels, bufsize = fbufsize, i;
  float32_t *buf=NULL;

  if (nchannels < 1)
    nchannels = 1;
  else if (nchannels > MAXSFCHANS)
    nchannels = MAXSFCHANS;
  if (bufsize <= 0)
    bufsize = DEFBUFPERCHAN * nchannels;
  else if (bufsize < MINBUFSIZE)
    bufsize = MINBUFSIZE;
  else if (bufsize > MAXBUFSIZE)
    bufsize = MAXBUFSIZE;
  bufsize*=sizeof(float32_t);
  buf = getbytes(bufsize);
  if (!buf) return (0);

  x = (t_ambix_read *)pd_new(ambix_read_class);

  for (i = 0; i < nchannels; i++)
    outlet_new(&x->x_obj, gensym("signal"));
  x->x_noutlets = nchannels;
  x->x_bangout = outlet_new(&x->x_obj, &s_bang);
  pthread_mutex_init(&x->x_mutex, 0);
  pthread_cond_init(&x->x_requestcondition, 0);
  pthread_cond_init(&x->x_answercondition, 0);
  x->x_vecsize = DEFAULTVECSIZE;
  x->x_state = STATE_IDLE;
  x->x_clock = clock_new(x, (t_method)ambix_read_tick);
  x->x_canvas = canvas_getcurrent();
  x->x_sfchannels = 1;
  x->x_fd = -1;
  x->x_buf = buf;
  x->x_bufsize = bufsize;
  x->x_fifosize = x->x_fifohead = x->x_fifotail = x->x_requestcode = 0;
  pthread_create(&x->x_childthread, 0, ambix_read_child_main, x);
  return (x);
}

static void ambix_read_tick(t_ambix_read *x) {
  outlet_bang(x->x_bangout);
}

static t_int *ambix_read_perform(t_int *w) {
  t_ambix_read *x = (t_ambix_read *)(w[1]);
  int vecsize = x->x_vecsize, noutlets = x->x_noutlets, i, j;
  t_sample *fp;
  if (x->x_state == STATE_STREAM) {
    int wantframes, nchannels, sfchannels = x->x_sfchannels;
    pthread_mutex_lock(&x->x_mutex);
    wantframes = vecsize;
    while (
           !x->x_eof && x->x_fifohead >= x->x_fifotail &&
           x->x_fifohead < x->x_fifotail + wantframes-1) {
      pthread_cond_signal(&x->x_requestcondition);
      pthread_cond_wait(&x->x_answercondition, &x->x_mutex);
      /* resync local cariables -- bug fix thanks to Shahrokh */
      vecsize = x->x_vecsize;
      sfchannels = x->x_sfchannels;
      wantframes = vecsize;
    }
    if (x->x_eof && x->x_fifohead >= x->x_fifotail &&
        x->x_fifohead < x->x_fifotail + wantframes-1) {
      int xfersize;
      if (x->x_fileerror) {
        pd_error(x, "dsp: %s: %s", x->x_filename,
                 (x->x_fileerror == EIO ?
                  "unknown or bad header format" :
                  strerror(x->x_fileerror)));
      }
      clock_delay(x->x_clock, 0);
      x->x_state = STATE_IDLE;

      /* if there's a partial buffer left, copy it out. */
      xfersize = (x->x_fifohead - x->x_fifotail + 1);
      if (xfersize) {
#if 1
#warning transfer samples
#else
        soundfile_xferin_sample(sfchannels, noutlets, x->x_outvec, 0,
                                (unsigned char *)(x->x_buf + x->x_fifotail), xfersize,
                                bytespersample, bigendian, 1);
#endif
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

#if 1
#warning transfer samples
#else
    soundfile_xferin_sample(sfchannels, noutlets, x->x_outvec, 0,
                            (unsigned char *)(x->x_buf + x->x_fifotail), vecsize,
                            bytespersample, bigendian, 1);
#endif
    x->x_fifotail += wantframes;
    if (x->x_fifotail >= x->x_fifosize)
      x->x_fifotail = 0;
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
  if (x->x_state == STATE_STARTUP)
    x->x_state = STATE_STREAM;
  else pd_error(x, "ambix_read: start requested with no prior 'open'");
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
   open filename [skipframes headersize channels bytespersamp endianness]
   (if headersize is zero, header is taken to be automatically
   detected; thus, use the special "-1" to mean a truly headerless file.)
*/

static void ambix_read_open(t_ambix_read *x, t_symbol *s, int argc, t_atom *argv) {
  t_symbol *filesym = atom_getsymbolarg(0, argc, argv);
  t_float onsetframes = atom_getfloatarg(1, argc, argv);
  t_float channels = atom_getfloatarg(3, argc, argv);
  if (!*filesym->s_name)
    return;
  pthread_mutex_lock(&x->x_mutex);
  x->x_requestcode = REQUEST_OPEN;
  x->x_filename = filesym->s_name;
  x->x_fifotail = 0;
  x->x_fifohead = 0;
  x->x_onsetframes = (onsetframes > 0 ? onsetframes : 0);
  x->x_sfchannels = (channels >= 1 ? channels : 1);
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
  post("fd %d", x->x_fd);
  post("eof %d", x->x_eof);
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
    error("ambix_read_free: join failed");

  pthread_cond_destroy(&x->x_requestcondition);
  pthread_cond_destroy(&x->x_answercondition);
  pthread_mutex_destroy(&x->x_mutex);
  freebytes(x->x_buf, x->x_bufsize);
  clock_free(x->x_clock);
}

void ambix_read_tilde_setup(void) {
  ambix_read_class = class_new(gensym("ambix_read~"), (t_newmethod)ambix_read_new,
                               (t_method)ambix_read_free, sizeof(t_ambix_read), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addfloat(ambix_read_class, (t_method)ambix_read_float);
  class_addmethod(ambix_read_class, (t_method)ambix_read_start, gensym("start"), 0);
  class_addmethod(ambix_read_class, (t_method)ambix_read_stop, gensym("stop"), 0);
  class_addmethod(ambix_read_class, (t_method)ambix_read_dsp, gensym("dsp"), 0);
  class_addmethod(ambix_read_class, (t_method)ambix_read_open, gensym("open"),
                  A_GIMME, 0);
  class_addmethod(ambix_read_class, (t_method)ambix_read_print, gensym("print"), 0);
}

