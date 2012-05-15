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
#include <string.h>
#include <stdlib.h>

/* ------------------------ ambixread ----------------------------- */
#define MAXCHANS 64
#define MAXVECSIZE 128
/* READSIZE was 65538, but we are using frames, not bytes */

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

typedef struct _ambixread
{
  t_object x_obj;
  t_symbol*x_objname;
  t_canvas *x_canvas;
  ambix_t*x_ambix;
  ambix_info_t x_info;

  int x_ambioutlets;                      /* number of ambi audio outlets */
  int x_xtraoutlets;                      /* number of extra audio outlets */

  t_outlet *x_mtxout ;                    /* matrix outlet (in ambix-extended mode) */
  t_outlet *x_infout;                    /* bang-on-done and other-info outlet */

  t_sample**x_outvec;

  float32_t*x_ambibuffer, *x_xtrabuffer;

  int x_vecsize;
  int x_stream;

  int x_banged;
} t_ambixread;

static t_int *ambixread_perform(t_int *w) {
  t_ambixread *x = (t_ambixread *)(w[1]);

  int frames=x->x_vecsize;
  int gotframes=0;
  int i;

  for(i=0; i<x->x_ambioutlets+x->x_xtraoutlets; i++) {
    int frame;
    t_sample*vec=x->x_outvec[i]+gotframes;
    for(frame=gotframes; frame<frames; frame++) {
      *vec++=0.f;
    }
  }


  if(x->x_stream && x->x_ambix) {
    int verbose=(x->x_banged);
    x->x_banged=0;

    

    ambix_t*ambix=x->x_ambix;
    float32_t*ambibuffer=x->x_ambibuffer;
    float32_t*xtrabuffer=x->x_xtrabuffer;

    gotframes=ambix_readf_float32(ambix, ambibuffer, xtrabuffer, frames);
    if(verbose)post("readf_float(%p, %p, %p, %d)=%d", ambix, ambibuffer, xtrabuffer, frames, gotframes);

    if(verbose) {
      float32_t*buf;
      uint32_t chan, fram;
      uint32_t channels;
      if(ambibuffer) {
        buf=ambibuffer;
        channels=x->x_info.ambichannels;
        printf("ambix: %p...%d*%d\n", buf, channels, gotframes);
        for(fram=0; fram<gotframes; fram++) {
          for(chan=0; chan<channels; chan++)
            printf("  %+05g", *buf++);
          printf("\n");
        }
      }
      /*
      if(xtrabuffer) {
        buf=xtrabuffer;
        channels=x->x_info.extrachannels;
        printf("extra: %p...%d\n", buf, channels);
        for(fram=0; fram<gotframes; fram++) {
          for(chan=0; chan<channels; chan++)
            printf("  %05g", *buf++);
          printf("\n");
        }
      }
      */
    }

    if(x->x_ambioutlets) {
      int frame=0;
      int channels=x->x_info.ambichannels;
      if(channels>x->x_ambioutlets)
        channels=x->x_ambioutlets;
      if(verbose)post("ambi: %d -> %d ... %d", x->x_info.ambichannels, x->x_ambioutlets, channels);
      for(frame=0; frame<gotframes; frame++) {
        for(i=0; i<channels; i++) {
          x->x_outvec[i][frame]=ambibuffer[frame*x->x_info.ambichannels+i];
        }
      }
    }
    if(x->x_xtraoutlets) {
      int frame=0;
      int channels=x->x_info.extrachannels;
      if(channels>x->x_xtraoutlets)
        channels=x->x_xtraoutlets;
      if(verbose)post("xtra: %d -> %d ... %d", x->x_info.extrachannels, x->x_xtraoutlets, channels);
      for(frame=0; frame<gotframes; frame++) {
        for(i=0; i<channels; i++) {
          x->x_outvec[i][frame]=xtrabuffer[frame*x->x_info.extrachannels+i];
        }
      }      
    }
  }


  return (w+2);
}
static void ambixread_dsp(t_ambixread *x, t_signal **sp) {
  int i, noutlets = x->x_ambioutlets+x->x_xtraoutlets;
  x->x_vecsize = sp[0]->s_n;

  x->x_ambibuffer=realloc(x->x_ambibuffer, sizeof(float32_t)*x->x_info.ambichannels*x->x_vecsize);
  x->x_xtrabuffer=realloc(x->x_xtrabuffer, sizeof(float32_t)*x->x_info.extrachannels*x->x_vecsize);

  for (i = 0; i < noutlets; i++)
    x->x_outvec[i] = sp[i]->s_vec;

  dsp_add(ambixread_perform, 1, x);
}

static void ambixread_start(t_ambixread *x) {
  if(x->x_ambix) {
    x->x_stream=1;
  } else {
    pd_error(x, "start without prior open");
    x->x_stream=0;
  }
}
static void ambixread_stop(t_ambixread *x) {
  x->x_stream=0;

  if(x->x_ambix)
    ambix_close(x->x_ambix);
  x->x_ambix=NULL;
}
static void ambixread_float(t_ambixread *x, t_floatarg f) {
  if (f != 0)
    ambixread_start(x);
  else ambixread_stop(x);
}
static void ambixread_open(t_ambixread *x, t_symbol *s, int argc, t_atom *argv) {
  ambix_fileformat_t format=(NULL!=x->x_mtxout)?AMBIX_EXTENDED:AMBIX_BASIC;
  ambixread_stop(x);
  const char*filename=atom_getsymbol(argv)->s_name;

  memset(&x->x_info, 0, sizeof(x->x_info));
  x->x_info.fileformat=format;
  x->x_ambix=ambix_open(filename, AMBIX_READ, &x->x_info);
  post("ambix open of '%s' got: %p --> %d", filename, x->x_ambix, format);
  if(x->x_ambix) {
    ambix_t*ambix=x->x_ambix;
    if(x->x_mtxout) {
      const ambix_matrix_t*mtx=ambix_get_adaptormatrix(ambix);
      if(mtx) {
        t_atom*atoms=calloc(mtx->rows*mtx->cols+2, sizeof(t_atom));
        t_atom*ap=atoms+2;
        uint32_t c=0, r=0;
        SETFLOAT(atoms+0, mtx->rows);
        SETFLOAT(atoms+1, mtx->cols);
        for(r=0; r<mtx->rows; r++)
          for(c=0; c<mtx->cols; c++) {
            SETFLOAT(ap, mtx->data[r][c]);
            ap++;
          }

        outlet_anything(x->x_mtxout, gensym("matrix"), mtx->rows*mtx->cols+2, atoms);
        free(atoms);
      }
    }
    x->x_ambibuffer=realloc(x->x_ambibuffer, sizeof(float32_t)*x->x_info.ambichannels*x->x_vecsize);
    x->x_xtrabuffer=realloc(x->x_xtrabuffer, sizeof(float32_t)*x->x_info.extrachannels*x->x_vecsize);
  }
}

static void ambixread_bang(t_ambixread *x) {
  x->x_banged=1;
}

static void ambixread_free(t_ambixread *x) {
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

#if 0
  if (bufsize <= 0) bufsize = DEFBUFPERCHAN;
  else if (bufsize < MINBUFSIZE)
    bufsize = MINBUFSIZE;
  else if (bufsize > MAXBUFSIZE)
    bufsize = MAXBUFSIZE;
#endif

  x->x_ambioutlets=ambichannels;
  for (i = 0; i < ambichannels; i++)
    outlet_new(&x->x_obj, gensym("signal"));

  x->x_xtraoutlets=extrachannels;
  for (i = 0; i < extrachannels; i++)
    outlet_new(&x->x_obj, gensym("signal"));

  x->x_outvec=calloc(ambichannels+extrachannels, sizeof(t_sample));

  x->x_infout = outlet_new(&x->x_obj, NULL);

  x->x_stream=0;
  x->x_ambix=NULL;

  x->x_ambibuffer=NULL;
  x->x_xtrabuffer=NULL;
  x->x_vecsize = 64;

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

  class_addmethod(ambixread_class, (t_method)ambixread_bang, gensym("bang"), 0);

}
void ambix_readX_tilde_setup(void) {
  ambix_read_tilde_setup();
}
