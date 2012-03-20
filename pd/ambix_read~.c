/* ambix_read~ -  read AMBIsonics Xchange files in Pd              -*- c -*-

Copyright © 2012 IOhannes m zmölnig <zmoelnig@iem.at>.
Institute of Electronic Music and Acoustics (IEM),
University of Music and Dramatic Arts, Graz

This file is part of libambix

libambix is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of
the License, or (at your option) any later version.

Libgcrypt is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this program; if not, see <http://www.gnu.org/licenses/>.

*/

#include "m_pd.h"
#include <ambix/ambix.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

/* ------------------------ ambixread ----------------------------- */
#define MAXCHANS 64
#define MAXVECSIZE 128
/* READSIZE was 65538, but we are using frames, not bytes */

#if 0
#define READSIZE 65536
#define DEFBUFPERCHAN 262144
#define MINBUFSIZE (4 * READSIZE)
#define MAXBUFSIZE 16777216     /* arbitrary; just don't want to hang malloc */
#endif
#define READSIZE 128
#define DEFBUFPERCHAN (4 * READSIZE)
#define MINBUFSIZE (4 * READSIZE)
#define MAXBUFSIZE (256 * READSIZE)     /* arbitrary; just don't want to hang malloc */

static t_class *ambixread_class;

typedef enum {
  NOTHING = 0,
  OPEN,
  CLOSE,
  QUIT,
  BUSY
} t_ambixrequest;
typedef enum {
  IDLE = 0,
  STARTUP,
  STREAM
} t_ambixstate;

/* parameters to communicate with subthread */
typedef struct _ambixthreaddata {
  t_ambixstate state;                            /* opened, running, or idle */
  t_ambixrequest requestcode;      /* pending request from parent to I/O thread */

  int vecsize;                          /* vector size for transfers */
  int bufsize;                          /* buffer size in bytes */

  ambix_t  *ambix;    /* handler for ambix */
  char *filename;       /* file to open (string is permanently allocated) */
  int fileerror;        /* slot for "errno" return */
  int skipframes;       /* size of header we'll skip */
  int ambichannels;     /* number of channels in soundfile */
  int xtrachannels;     /* number of channels in soundfile */
  t_float samplerate;   /* sample rate of soundfile */

  long onsetframes;     /* number of sample frames to skip */


  int fifosize;         /* buffer size appropriately rounded down */            
  int fifohead;         /* index of next byte to get from file */
  int fifotail;         /* index of next byte the ugen will read */

  float32_t*ambififo;   /* fifo for the ambisonics data, of size ambichannels*fifosize */
  float32_t*xtrafifo;   /* fifo for the exxxxxxtra data, of size xtrachannels*fifosize */

  int eof;              /* true if fifohead has stopped changing */
  int sigcountdown;     /* counter for signalling child for more data */
  int sigperiod;        /* number of ticks per signal */

  pthread_mutex_t mutex;
  pthread_cond_t requestcondition;
  pthread_cond_t answercondition;
} t_ambixthreaddata;

typedef struct _ambixread
{
  t_object x_obj;
  t_symbol*x_objname;
  t_canvas *x_canvas;
  t_clock *x_clock;

  int x_ambioutlets;                      /* number of ambi audio outlets */
  int x_xtraoutlets;                      /* number of extra audio outlets */

  t_outlet *x_mtxout ;                    /* matrix outlet (in ambix-extended mode) */
  t_outlet *x_infout;                    /* bang-on-done and other-info outlet */

  t_sample *(x_outvec[MAXCHANS]);         /* audio vectors */

  t_ambixthreaddata x_threaddata;
  pthread_t x_childthread;
} t_ambixread;

static void *ambixread_thread(void *zz) {
  t_ambixthreaddata *x = zz;
  pthread_mutex_lock(&x->mutex);
  while (1) {
    ambix_t*ambix;
    ambix_info_t info;
    int fifohead;
    char *buf;
    if (x->requestcode == NOTHING) {
      sfread_cond_signal(&x->answercondition);
      sfread_cond_wait(&x->requestcondition, &x->mutex);
    } else if (x->requestcode == OPEN) {
      int  wantframes;
            
      /* copy file stuff out of the data structure so we can
         relinquish the mutex while we're in open_soundfile(). */
      uint64_t onsetframes = x->onsetframes;
      char *filename = x->filename;
      uint64_t fifosize = x->fifosize;

      float32_t*ambififo,*xtrafifo;

      /* alter the request code so that an ensuing "open" will get
         noticed. */
      x->requestcode = BUSY;
      x->fileerror = 0;

      /* if there's already a file open, close it */
      if (NULL != x->ambix) {
        ambix=x->ambix;
        pthread_mutex_unlock(&x->mutex);
        ambix_close (ambix);
        pthread_mutex_lock(&x->mutex);
        x->ambix=NULL;
        if (x->requestcode != BUSY)
          goto lost;
      }
      /* open the soundfile with the mutex unlocked */
      pthread_mutex_unlock(&x->mutex);
      memset(&info, 0, sizeof(info));
#warning request proper ambixtype
      info.fileformat=AMBIX_SIMPLE;
      ambix = ambix_open(filename, AMBIX_READ, &info);
      if (ambix != NULL) {
        ambififo = calloc(info.ambichannels *fifosize, sizeof(float32_t));
        xtrafifo = calloc(info.extrachannels*fifosize, sizeof(float32_t));
      }

      pthread_mutex_lock(&x->mutex);

      /* copy back into the instance structure. */
      x->ambix = ambix;
      if (ambix == NULL) {
        x->fileerror = 10;
        x->eof = 1;
        goto lost;
      }
      x->ambichannels = info.ambichannels;
      x->xtrachannels = info.extrachannels;
      free(x->ambififo);
      free(x->xtrafifo);
      x->ambififo=ambififo;
      x->xtrafifo=xtrafifo;

      /* check if another request has been made; if so, field it */
      if (x->requestcode != BUSY)
        goto lost;

      x->fifohead = 0;
      /* set fifosize from bufsize.  fifosize must be a
         multiple of the number of bytes eaten for each DSP
         tick.  We pessimistically assume MAXVECSIZE samples
         per tick since that could change.  There could be a
         problem here if the vector size increases while a
         soundfile is being played...  */
      x->fifosize = x->bufsize - (x->bufsize % MAXVECSIZE);
      /* arrange for the "request" condition to be signalled 16
         times per buffer */

      x->sigcountdown = x->sigperiod =
        (x->fifosize /
         (16 * x->vecsize));

      /* in a loop, wait for the fifo to get hungry and feed it */

      while (x->requestcode == BUSY) {
        uint64_t fifosize = x->fifosize;
        uint64_t gotframes=0;
        float32_t*ambififo, *xtrafifo;
        uint32_t ambichannels, xtrachannels;
        if (x->eof)
          break;
        if (x->fifohead >= x->fifotail) {
          /* if the head is >= the tail, we can immediately read
             to the end of the fifo.  Unless, that is, we would
             read all the way to the end of the buffer and the 
             "tail" is zero; this would fill the buffer completely
             which isn't allowed because you can't tell a completely
             full buffer from an empty one. */
          if (x->fifotail || (fifosize - x->fifohead > READSIZE)) {
            wantframes = fifosize - x->fifohead;
            if (wantframes > READSIZE)
              wantframes = READSIZE;
          } else {
            pthread_cond_signal(&x->answercondition);
            pthread_cond_wait(&x->requestcondition,
                             &x->mutex);
            continue;
          }
        } else {
          /* otherwise check if there are at least READSIZE
             bytes to read.  If not, wait and loop back. */
          wantframes =  x->fifotail - x->fifohead - 1;
          if (wantframes < READSIZE) {
            pthread_cond_signal(&x->answercondition);
            pthread_cond_wait(&x->requestcondition,
                             &x->mutex);
            continue;
          }
          else wantframes = READSIZE;
        }
        ambix = x->ambix;
        fifohead = x->fifohead;
        ambififo = x->ambififo;
        ambichannels = x->ambichannels;
        xtrafifo = x->xtrafifo;
        xtrachannels = x->xtrachannels;
        pthread_mutex_unlock(&x->mutex);
        gotframes=ambix_readf_float32(ambix, 
                                      ambififo + fifohead*ambichannels,
                                      xtrafifo + fifohead*xtrachannels,
                                      wantframes);

        pthread_mutex_lock(&x->mutex);
        if (x->requestcode != BUSY)
          break;
        if (gotframes < 0) {
          x->fileerror = 11;
          break;
        } else if (gotframes == 0) {
          x->eof = 1;
          break;
        } else {
          x->fifohead += gotframes;
          if (x->fifohead == fifosize)
            x->fifohead = 0;
        }
        /* signal parent in case it's waiting for data */
        pthread_cond_signal(&x->answercondition);
      }
    lost:

      if (x->requestcode == BUSY)
        x->requestcode = NOTHING;
      /* fell out of read loop: close file if necessary,
         set EOF and signal once more */
      if (NULL!=x->ambix) {
        ambix=x->ambix;
        pthread_mutex_unlock(&x->mutex);
        ambix_close (ambix);
        pthread_mutex_lock(&x->mutex);
        x->ambix = NULL;
      }
      pthread_cond_signal(&x->answercondition);

    } else if (x->requestcode == CLOSE) {
      if (NULL!=x->ambix) {
        ambix=x->ambix;
        pthread_mutex_unlock(&x->mutex);
        ambix_close (ambix);
        pthread_mutex_lock(&x->mutex);
        x->ambix = NULL;
      }
      if (x->requestcode == CLOSE)
        x->requestcode = NOTHING;
      pthread_cond_signal(&x->answercondition);
    } else if (x->requestcode == QUIT) {
      if (NULL!=x->ambix) {
        ambix=x->ambix;
        pthread_mutex_unlock(&x->mutex);
        ambix_close (ambix);
        pthread_mutex_lock(&x->mutex);
        x->ambix = NULL;
      }
      x->requestcode = NOTHING;
      pthread_cond_signal(&x->answercondition);
      break;
    } else {
    }
  }
  pthread_mutex_unlock(&x->mutex);
  return (0);
  return NULL;
}

static t_int *ambixread_perform(t_int *w) {
  t_ambixread *x = (t_ambixread *)(w[1]);
  int vecsize = x->x_threaddata.vecsize;
  int ambioutlets = x->x_ambioutlets;
  int xtraoutlets = x->x_xtraoutlets;
  int noutlets = ambioutlets+xtraoutlets;
  int i, j;
  t_sample *fp;

  if (x->x_threaddata.state == STREAM) {
    int wantframes;
    float32_t*ambidata;
    float32_t*xtradata;
    pthread_mutex_lock(&x->x_threaddata.mutex);
    wantframes = vecsize;
    while (
           !x->x_threaddata.eof && x->x_threaddata.fifohead >= x->x_threaddata.fifotail &&
           x->x_threaddata.fifohead < x->x_threaddata.fifotail + wantframes-1)  {
      pthread_cond_signal(&x->x_threaddata.requestcondition);
      pthread_cond_wait(&x->x_threaddata.answercondition, &x->x_threaddata.mutex);

      wantframes = vecsize;
    }
    ambidata=x->x_threaddata.ambififo + x->x_threaddata.fifotail*x->x_threaddata.ambichannels;
    xtradata=x->x_threaddata.xtrafifo + x->x_threaddata.fifotail*x->x_threaddata.xtrachannels;

    if (x->x_threaddata.eof && x->x_threaddata.fifohead >= x->x_threaddata.fifotail &&
        x->x_threaddata.fifohead < x->x_threaddata.fifotail + wantframes-1) {
      int xfersize;
      if (x->x_threaddata.fileerror) {
#if 0
        pd_error(x, "dsp: %s: %s", x->x_threaddata.filename,
                 (x->x_threaddata.fileerror == EIO ?
                  "unknown or bad header format" :
                  strerror(x->x_threaddata.fileerror)));
#else
        pd_error(x, "dsp: %s: file error %d", x->x_threaddata.filename,x->x_threaddata.fileerror);
#endif
      }
      clock_delay(x->x_clock, 0);
      x->x_threaddata.state = IDLE;

      /* if there's a partial buffer left, copy it out. */      
      xfersize = (x->x_threaddata.fifohead - x->x_threaddata.fifotail + 1);
      if (xfersize) {
        for(i=0; i<xfersize; i++) {
          for(j=0; j<ambioutlets; j++)
            x->x_outvec[j][i] = *ambidata++;
          for(j=0; j<xtraoutlets; j++)
            x->x_outvec[j+ambioutlets][i] = *xtradata++;
        }
        vecsize -= xfersize;
      }
      /* then zero out the (rest of the) output */
      for (i = 0; i < noutlets; i++)
        for (j = vecsize, fp = x->x_outvec[i]; j--; )
          *fp++ = 0;

      pthread_cond_signal(&x->x_threaddata.requestcondition);
      pthread_mutex_unlock(&x->x_threaddata.mutex);
      return (w+2); 
    }

    /* streaming, get data */
    for(i=0; i<vecsize; i++) {
      for(j=0; j<ambioutlets; j++)
        x->x_outvec[j][i] = *ambidata++;
      for(j=0; j<xtraoutlets; j++)
        x->x_outvec[j+ambioutlets][i] = *xtradata++;
    }

    x->x_threaddata.fifotail += wantframes;
    if (x->x_threaddata.fifotail >= x->x_threaddata.fifosize)
      x->x_threaddata.fifotail = 0;
    if ((--x->x_threaddata.sigcountdown) <= 0) {
      pthread_cond_signal(&x->x_threaddata.requestcondition);
      x->x_threaddata.sigcountdown = x->x_threaddata.sigperiod;
    }
    pthread_mutex_unlock(&x->x_threaddata.mutex);
  } else {
    /* not streaming, fill with zeros */
    for (i = 0; i < noutlets; i++)
      for (j = vecsize, fp = x->x_outvec[i]; j--; )
        *fp++ = 0;
  }
  return (w+2);
}
static void ambixread_dsp(t_ambixread *x, t_signal **sp) {
  int i, noutlets = x->x_ambioutlets+x->x_xtraoutlets;
  pthread_mutex_lock(&x->x_threaddata.mutex);
  x->x_threaddata.vecsize = sp[0]->s_n;
    
  x->x_threaddata.sigperiod = (x->x_threaddata.fifosize / x->x_threaddata.vecsize);
  for (i = 0; i < noutlets; i++)
    x->x_outvec[i] = sp[i]->s_vec;
  pthread_mutex_unlock(&x->x_threaddata.mutex);
  dsp_add(ambixread_perform, 1, x);
}

static void ambixread_start(t_ambixread *x) {
  /* start making output.  If we're in the "startup" state change
     to the "running" state. */
  if (x->x_threaddata.state == STARTUP)
    x->x_threaddata.state = STREAM;
  else pd_error(x, "%s: start requested with no prior 'open'", x->x_objname->s_name);
}
static void ambixread_stop(t_ambixread *x) {
  /* LATER rethink whether you need the mutex just to set a variable? */
  pthread_mutex_lock(&x->x_threaddata.mutex);
  x->x_threaddata.state = IDLE;
  x->x_threaddata.requestcode = CLOSE;
  pthread_cond_signal(&x->x_threaddata.requestcondition);
  pthread_mutex_unlock(&x->x_threaddata.mutex);
}
static void ambixread_float(t_ambixread *x, t_floatarg f) {
  if (f != 0)
    ambixread_start(x);
  else ambixread_stop(x);
}
static void ambixread_open(t_ambixread *x, t_symbol *s, int argc, t_atom *argv) {

}
static void ambixread_tick(t_ambixread *x) {
  post("tick!");
}
static void ambixread_free(t_ambixread *x) {
  /* request QUIT and wait for acknowledge */
  void *threadrtn;
  pthread_mutex_lock(&x->x_threaddata.mutex);
  x->x_threaddata.requestcode = QUIT;
  pthread_cond_signal(&x->x_threaddata.requestcondition);
  while (x->x_threaddata.requestcode != NOTHING)
    {
      pthread_cond_signal(&x->x_threaddata.requestcondition);
      pthread_cond_wait(&x->x_threaddata.answercondition, &x->x_threaddata.mutex);
    }
  pthread_mutex_unlock(&x->x_threaddata.mutex);
  if (pthread_join(x->x_childthread, &threadrtn))
    error("readsf_free: join failed");
    
  pthread_cond_destroy(&x->x_threaddata.requestcondition);
  pthread_cond_destroy(&x->x_threaddata.answercondition);
  pthread_mutex_destroy(&x->x_threaddata.mutex);

  free(x->x_threaddata.ambififo);
  free(x->x_threaddata.xtrafifo);

  clock_free(x->x_clock);
}
/*  */
static void *ambixread_new(t_symbol*s, int argc, t_atom*argv) {
  uint32_t ambichannels = 4;
  uint32_t extrachannels = 2;
  uint64_t bufsize = 0;
  uint32_t i;
  t_ambixread*x=(t_ambixread*)pd_new(ambixread_class);
  x->x_objname=s;
  if(gensym("ambix_readX~") == s)
    x->x_mtxout = outlet_new(&x->x_obj, gensym("matrix"));

  if(ambichannels+extrachannels>MAXCHANS) {
    /* ouch, user requested too much! */
    if(ambichannels>MAXCHANS) {
      ambichannels=MAXCHANS;
      extrachannels=0;      
    } else {
      extrachannels=MAXCHANS-ambichannels;
    }
    pd_error(x, "limiting to %d ambisonics channels and %d extrachannels", ambichannels, extrachannels);
  }

  if (bufsize <= 0) bufsize = DEFBUFPERCHAN;
  else if (bufsize < MINBUFSIZE)
    bufsize = MINBUFSIZE;
  else if (bufsize > MAXBUFSIZE)
    bufsize = MAXBUFSIZE;


  x->x_ambioutlets=ambichannels;
  for (i = 0; i < ambichannels; i++)
    outlet_new(&x->x_obj, gensym("signal"));

  x->x_xtraoutlets=extrachannels;
  for (i = 0; i < extrachannels; i++)
    outlet_new(&x->x_obj, gensym("signal"));

  x->x_infout = outlet_new(&x->x_obj, NULL);

  x->x_clock = clock_new(x, (t_method)ambixread_tick);
  x->x_canvas = canvas_getcurrent();

  memset(&x->x_threaddata, 0, sizeof(x->x_threaddata));

  pthread_mutex_init(&x->x_threaddata.mutex, 0);
  pthread_cond_init(&x->x_threaddata.requestcondition, 0);
  pthread_cond_init(&x->x_threaddata.answercondition, 0);

  x->x_threaddata.vecsize = MAXVECSIZE;
  x->x_threaddata.state = IDLE;
  x->x_threaddata.fifosize = x->x_threaddata.fifohead = x->x_threaddata.fifotail = x->x_threaddata.requestcode = 0;
  pthread_create(&x->x_childthread, 0, ambixread_thread, &x->x_threaddata);

  x->x_threaddata.bufsize = bufsize;

  return x;
}

void ambix_read_tilde_setup(void) {
  ambixread_class = class_new(gensym("ambix_read~"), (t_newmethod)ambixread_new, 
                              (t_method)ambixread_free, sizeof(t_ambixread), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addcreator((t_newmethod)ambixread_new, gensym("ambix_readX~"), A_GIMME, 0);

  class_addfloat(ambixread_class, (t_method)ambixread_float);
  class_addmethod(ambixread_class, (t_method)ambixread_start, gensym("start"), 0);
  class_addmethod(ambixread_class, (t_method)ambixread_stop, gensym("stop"), 0);
  class_addmethod(ambixread_class, (t_method)ambixread_dsp, gensym("dsp"), 0);
  class_addmethod(ambixread_class, (t_method)ambixread_open, gensym("open"), 
                  A_GIMME, 0);
  //    class_addmethod(ambixread_class, (t_method)ambixread_print, gensym("print"), 0);
}
void ambix_readX_tilde_setup(void) {
  ambix_read_tilde_setup();
}
