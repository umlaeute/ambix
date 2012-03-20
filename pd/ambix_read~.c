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

/* ------------------------ ambixread ----------------------------- */

static t_class *ambixread_class;

typedef struct _ambixread
{
  t_object x_obj;
  t_canvas *x_canvas;
  t_clock *x_clock;
  char *x_buf;                            /* soundfile buffer */
  int x_bufsize;                          /* buffer size in bytes */
  int x_ambioutlets;                      /* number of ambi audio outlets */
  int x_xtraoutlets;                      /* number of extra audio outlets */
  t_outlet *x_mtxout ;                    /* matrix outlet (in ambix-extended mode) */
  t_outlet *x_infoout;                    /* bang-on-done and other-info outlet */

  t_sample *(x_outvec[MAXSFCHANS]);       /* audio vectors */
  int x_vecsize;                          /* vector size for transfers */
  int x_state;                            /* opened, running, or idle */
  t_float x_insamplerate;   /* sample rate of input signal if known */
  /* parameters to communicate with subthread */
  int x_requestcode;      /* pending request from parent to I/O thread */
  char *x_filename;       /* file to open (string is permanently allocated) */
  int x_fileerror;        /* slot for "errno" return */
  int x_skipheaderbytes;  /* size of header we'll skip */
  int x_bytespersample;   /* bytes per sample (2 or 3) */
  int x_bigendian;        /* true if file is big-endian */
  int x_sfchannels;       /* number of channels in soundfile */
  t_float x_samplerate;     /* sample rate of soundfile */
  long x_onsetframes;     /* number of sample frames to skip */
  long x_bytelimit;       /* max number of data bytes to read */
  int x_fd;               /* filedesc */
  int x_fifosize;         /* buffer size appropriately rounded down */            
  int x_fifohead;         /* index of next byte to get from file */
  int x_fifotail;         /* index of next byte the ugen will read */
  int x_eof;              /* true if fifohead has stopped changing */
  int x_sigcountdown;     /* counter for signalling child for more data */
  int x_sigperiod;        /* number of ticks per signal */
  int x_filetype;         /* writesf~ only; type of file to create */
  int x_itemswritten;     /* writesf~ only; items writen */
  int x_swap;             /* writesf~ only; true if byte swapping */
  t_float x_f;              /* writesf~ only; scalar for signal inlet */
  pthread_mutex_t x_mutex;
  pthread_cond_t x_requestcondition;
  pthread_cond_t x_answercondition;
  pthread_t x_childthread;
} t_ambixread;

static t_int *ambixread_perform(t_int *w) {

}
static void ambixread_dsp(t_ambixread *x, t_signal **sp) {

}

static void ambixread_start(t_ambixread *x) {

}
static void ambixread_stop(t_ambixread *x) {

}
static void ambixread_float(t_ambixread *x, t_floatarg f) {
  if (f != 0)
    ambixread_start(x);
  else ambixread_stop(x);
}
static void ambixread_open(t_ambixread *x, t_symbol *s, int argc, t_atom *argv) {

}
static void ambixread_free(t_ambixread *x) {

}
static void *ambixread_new(t_symbol*s, int argc, t_atom*argv) {

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
    class_addmethod(ambixread_class, (t_method)ambixread_print, gensym("print"), 0);
}
void ambix_readX_tilde_setup(void) {
  ambix_read_tilde_setup();
}
