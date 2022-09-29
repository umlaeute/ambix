/* ambix_info -  get information on AMBIsonics eXchange files into Pd        -*- c -*-

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

*/

#if 0
# define MARK printf("%s[%d]:%s\t", __FILE__, __LINE__, __FUNCTION__), printf
#else
static void noop(const char*format, ...) {}
# define MARK noop
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <m_pd.h>
#include <ambix/ambix.h>

#include "winhacks.h"

/************************* ambix_info object ******************************/

/* [ambix_info~] uses the Posix threads package; for the moment we're Linux
   only although this should be portable to the other platforms.

   Each instance of ambix_info~ owns a "child" thread for doing the unix (MSW?) file
   reading.  The parent thread signals the child each time:
   (1) a file wants opening or closing;
   (2) we've eaten another 1/16 of the shared buffer (so that the
   child thread should check if it's time to read some more.)
   The child signals the parent whenever a read has completed.  Signalling
   is done by setting "conditions" and putting data in mutex-controlled common
   areas.
*/

static t_class *ambix_info_class;

typedef struct _ambix_info {
  t_object x_obj;
  t_canvas *x_canvas;

  t_outlet*x_outlet;
} t_ambix_info;


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


/******** the object proper runs in the calling (parent) thread ****/

static void *ambix_info_new(void) {
  t_ambix_info*x = (t_ambix_info *)pd_new(ambix_info_class);
  x->x_outlet = outlet_new(&x->x_obj, &s_bang);
  x->x_canvas = canvas_getcurrent();
  return (x);
}

static void ambix_info_open(t_ambix_info *x, t_symbol*s) {
  t_atom atoms[1];

  ambix_info_t ainfo;
  ambix_t*ambix=NULL;
  const ambix_matrix_t*matrix=NULL;

  t_symbol*filesym=get_filename(x->x_canvas, s);
  const char*filename=(filesym!=NULL)?filesym->s_name:NULL;

  memset(&ainfo, 0, sizeof(ainfo));

  ambix=ambix_open(filename, AMBIX_READ, &ainfo);

  if(!ambix) {
    pd_error(x, "[ambix_info]: unable to open '%s'", filename);
    return;
  }


  /* filename */
  SETSYMBOL(atoms+0, filesym);
  outlet_anything(x->x_outlet, gensym("filename"), 1, atoms);

  /* ambix fileformat */
  switch(ainfo.fileformat) {
  case AMBIX_NONE: SETSYMBOL(atoms+0, gensym("NONE")); break;
  case AMBIX_BASIC: SETSYMBOL(atoms+0, gensym("BASIC")); break;
  case AMBIX_EXTENDED: SETSYMBOL(atoms+0, gensym("EXTENDED")); break;
  default: SETSYMBOL(atoms+0, gensym("unknown")); break;
  }
  outlet_anything(x->x_outlet, gensym("fileformat"), 1, atoms);

  /* number of ambisonics channels */
  SETFLOAT(atoms+0, (t_float)(ainfo.ambichannels));
  outlet_anything(x->x_outlet, gensym("ambichannels"), 1, atoms);

  /* number of non-ambisonics channels */
  SETFLOAT(atoms+0, (t_float)(ainfo.extrachannels));
  outlet_anything(x->x_outlet, gensym("extrachannels"), 1, atoms);

  /* ambix sampleformat */
  switch(ainfo.sampleformat) {
  case AMBIX_SAMPLEFORMAT_NONE: SETSYMBOL(atoms+0, gensym("NONE")); break;
  case AMBIX_SAMPLEFORMAT_PCM16: SETSYMBOL(atoms+0, gensym("PCM16")); break;
  case AMBIX_SAMPLEFORMAT_PCM24: SETSYMBOL(atoms+0, gensym("PCM24")); break;
  case AMBIX_SAMPLEFORMAT_PCM32: SETSYMBOL(atoms+0, gensym("PCM32")); break;
  case AMBIX_SAMPLEFORMAT_FLOAT32: SETSYMBOL(atoms+0, gensym("FLOAT32")); break;
  default: SETSYMBOL(atoms+0, gensym("unknown")); break;
  }
  outlet_anything(x->x_outlet, gensym("sampleformat"), 1, atoms);

  /* samplerate of file */
  SETFLOAT(atoms+0, (t_float)(ainfo.samplerate));
  outlet_anything(x->x_outlet, gensym("samplerate"), 1, atoms);

  /* number of sample frames in file */
  SETFLOAT(atoms+0, (t_float)(ainfo.frames));
  outlet_anything(x->x_outlet, gensym("frames"), 1, atoms);

  matrix=ambix_get_adaptormatrix(ambix);

  if(matrix) {
    int size=matrix->rows*matrix->cols;
    if(size) {
      uint32_t r, c, index;
      t_atom*ap=(t_atom*)getbytes(sizeof(t_atom)*(size+2));
      SETFLOAT(ap+0, matrix->rows);
      SETFLOAT(ap+1, matrix->cols);

      index=2;
      for(r=0; r<matrix->rows; r++) {
        for(c=0; c<matrix->cols; c++) {
          SETFLOAT(ap+index, matrix->data[r][c]);
          index++;
        }
      }

      outlet_anything(x->x_outlet, gensym("matrix"), size+2, ap);
      freebytes(ap, sizeof(t_atom)*(size+2));
    }
  }
  ambix_close(ambix);
}

static void ambix_info_free(t_ambix_info *x) {
  /* request QUIT and wait for acknowledge */
  outlet_free(x->x_outlet);
}

AMBIX_EXPORT
void ambix_info_setup(void) {
  ambix_info_class = class_new(gensym("ambix_info"), (t_newmethod)ambix_info_new,
                               (t_method)ambix_info_free, sizeof(t_ambix_info), 0, A_GIMME, A_NULL);

  class_addmethod(ambix_info_class, (t_method)ambix_info_open, gensym("open"), A_SYMBOL, A_NULL);
  if(0) {
    MARK("setup");
  }
}
