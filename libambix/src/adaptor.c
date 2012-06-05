/* adaptor.c -  extracting ambisonics data from data using adaptor matrices              -*- c -*-

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

#include "private.h"

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */

ambix_err_t _ambix_adaptorbuffer_resize(ambix_t*ambix, uint64_t frames, uint16_t itemsize) {
  uint32_t channels=ambix->info.ambichannels + ambix->info.extrachannels;

  uint64_t size=channels*frames*itemsize;

  if(frames<1 || channels<1)
    return AMBIX_ERR_SUCCESS;
  if(size<1)
    return AMBIX_ERR_UNKNOWN;

  if(size > ambix->adaptorbuffersize) {
    /* re-allocate memory! */
    ambix->adaptorbuffer=realloc(ambix->adaptorbuffer, size);
    if(ambix->adaptorbuffer)
      ambix->adaptorbuffersize=size;
    else {
      ambix->adaptorbuffersize=0;
      return AMBIX_ERR_UNKNOWN;
    }
  }
  return AMBIX_ERR_SUCCESS;
}

ambix_err_t _ambix_adaptorbuffer_destroy(ambix_t*ambix) {
  if(ambix->adaptorbuffer)
    free(ambix->adaptorbuffer);
  ambix->adaptorbuffer=NULL;
  ambix->adaptorbuffersize=0;
  return AMBIX_ERR_SUCCESS;
}


#define _AMBIX_SPLITADAPTOR(type) \
  ambix_err_t _ambix_splitAdaptor_##type(const type##_t*source, uint32_t sourcechannels, \
                                         uint32_t ambichannels, type##_t*dest_ambi, type##_t*dest_other, \
                                         int64_t frames) {              \
    int64_t frame;                                                      \
    for(frame=0; frame<frames; frame++) {                               \
      uint32_t chan;                                                    \
      for(chan=0; chan<ambichannels; chan++)                            \
        *dest_ambi++=*source++;                                         \
      for(chan=ambichannels; chan<sourcechannels; chan++)               \
        *dest_other++=*source++;                                        \
    }                                                                   \
    return AMBIX_ERR_SUCCESS;                                           \
  }

_AMBIX_SPLITADAPTOR(float32);
_AMBIX_SPLITADAPTOR(int32);
_AMBIX_SPLITADAPTOR(int16);



#define _AMBIX_SPLITADAPTOR_MATRIX(type)                                \
  ambix_err_t _ambix_splitAdaptormatrix_##type(const type##_t*source, uint32_t sourcechannels, \
                                               const ambix_matrix_t*matrix, \
                                               type##_t*dest_ambi, type##_t*dest_other, \
                                               int64_t frames) {        \
    float32_t**mtx=matrix->data;                                        \
    const uint32_t fullambichannels=matrix->rows;                       \
    const uint32_t rawambichannels=matrix->cols;                        \
    int64_t f;                                                          \
    for(f=0; f<frames; f++) {                                           \
      uint32_t outchan, inchan;                                         \
      const type##_t*src = source+sourcechannels*f;                     \
      for(outchan=0; outchan<fullambichannels; outchan++) {             \
        float32_t sum=0.;                                               \
        for(inchan=0; inchan<rawambichannels; inchan++) {               \
          sum+=mtx[outchan][inchan] * src[inchan];                      \
        }                                                               \
		*dest_ambi++=(type##_t)sum;  /* FIXXXME: integer saturation */   \
      }                                                                 \
      for(inchan=rawambichannels; inchan<sourcechannels; inchan++)      \
        *dest_other++=src[inchan];                                      \
    }                                                                   \
    return AMBIX_ERR_SUCCESS;                                           \
  }

_AMBIX_SPLITADAPTOR_MATRIX(float32);
/* both _int16 and _int32 are highly unoptimized!
 * LATER: add some fixed point magic to speed things up
 */
_AMBIX_SPLITADAPTOR_MATRIX(int32);
_AMBIX_SPLITADAPTOR_MATRIX(int16);

#define _AMBIX_MERGEADAPTOR(type)                                       \
  ambix_err_t _ambix_mergeAdaptor_##type(const type##_t*source1, uint32_t source1channels, \
                                         const type##_t*source2, uint32_t source2channels, \
                                         type##_t*destination, int64_t frames) { \
    int64_t frame;                                                      \
    for(frame=0; frame<frames; frame++) {                               \
      uint32_t chan;                                                    \
      for(chan=0; chan<source1channels; chan++)                         \
        *destination++=*source1++;                                      \
      for(chan=0; chan<source2channels; chan++)                         \
        *destination++=*source2++;                                      \
    }                                                                   \
    return AMBIX_ERR_SUCCESS;                                           \
  }

_AMBIX_MERGEADAPTOR(float32);

_AMBIX_MERGEADAPTOR(int32);

_AMBIX_MERGEADAPTOR(int16);
