/* libambix.c -  Ambisonics Xchange Library              -*- c -*-

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

#include "private.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

ambix_t* 	ambix_open	(const char *path, const ambix_filemode_t mode, ambixinfo_t*ambixinfo) {
  ambix_t*ambix=calloc(1, sizeof(ambix_t));

  if(AMBIX_ERR_SUCCESS == _ambix_open(ambix, path, mode, ambixinfo)) {
    /* successfully opened, initialize common stuff... */
    if(_ambix_adaptorbuffer_resize(ambix, DEFAULT_ADAPTORBUFFER_SIZE, sizeof(float32_t)) == AMBIX_ERR_SUCCESS)
      return ambix;
  }

  ambix_close(ambix);
  return NULL;
}

ambix_err_t	ambix_close	(ambix_t*ambix) {
  int res=AMBIX_ERR_SUCCESS;
  if(NULL==ambix) {
    return AMBIX_ERR_INVALID_HANDLE;
  }
  res=_ambix_close(ambix);


  _ambix_adaptorbuffer_destroy(ambix);
  ambix_matrix_deinit(&ambix->matrix);

  free(ambix);
  ambix=NULL;
  return res;
}

SNDFILE*ambix_getSndfile	(ambix_t*ambix) {
  return _ambix_get_sndfile(ambix);
}


const ambixmatrix_t*ambix_getAdaptorMatrix	(ambix_t*ambix) {
  if(AMBIX_EXTENDED==ambix->info.ambixfileformat)
    return &(ambix->matrix);
  return NULL;
}
ambix_err_t ambix_setPremultiplyMatrix	(ambix_t*ambix, const ambixmatrix_t*matrix) {
  return AMBIX_ERR_UNKNOWN;
}

#define AMBIX_READF(type) \
  int64_t ambix_readf_##type (ambix_t*ambix, type##_t*ambidata, type##_t*otherdata, int64_t frames) { \
    int64_t realframes;                                                 \
    type##_t*adaptorbuffer;                                             \
    _ambix_adaptorbuffer_resize(ambix, frames, sizeof(type##_t));       \
    adaptorbuffer=(type##_t*)ambix->adaptorbuffer;                      \
    realframes=_ambix_readf_##type(ambix, adaptorbuffer, frames);       \
    if(0)                                                               \
      _ambix_splitAdaptormatrix_##type(adaptorbuffer, ambix->info.ambichannels+ambix->info.otherchannels, &ambix->matrix          , ambidata, otherdata, realframes); \
    else                                                                \
      _ambix_splitAdaptor_##type      (adaptorbuffer, ambix->info.ambichannels+ambix->info.otherchannels, ambix->info.ambichannels, ambidata, otherdata, realframes); \
    return realframes;                                                  \
  }

#define AMBIX_WRITEF(type) \
  int64_t ambix_writef_##type (ambix_t*ambix, type##_t *ambidata, type##_t*otherdata, int64_t frames) { \
    type##_t*adaptorbuffer;                                             \
    _ambix_adaptorbuffer_resize(ambix, frames, sizeof(type##_t));       \
    adaptorbuffer=(type##_t*)ambix->adaptorbuffer;                      \
    _ambix_mergeAdaptor_##type(ambidata, ambix->info.ambichannels, otherdata, ambix->info.otherchannels, adaptorbuffer, frames); \
    return _ambix_writef_##type(ambix, adaptorbuffer, frames);          \
  }


AMBIX_READF(int16);

AMBIX_READF(int32);

AMBIX_READF(float32);

AMBIX_WRITEF(int16);

AMBIX_WRITEF(int32);

AMBIX_WRITEF(float32);
