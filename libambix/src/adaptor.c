/* adaptor.c -  extracting ambisonics data from data using adaptor matrices              -*- c -*-

   Copyright © 2012-2016 IOhannes m zmölnig <zmoelnig@iem.at>.
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

static inline uint64_t max_u64(uint64_t a, uint64_t b) {
  return((a>b)?a:b);
}
ambix_err_t _ambix_adaptorbuffer_resize(ambix_t*ambix, uint64_t frames, uint16_t itemsize) {
  uint32_t ambichannels=max_u64(ambix->info.ambichannels,ambix->realinfo.ambichannels);
  uint32_t extrachannels=max_u64(ambix->info.extrachannels,ambix->realinfo.extrachannels);
  uint32_t channels=ambichannels + extrachannels;
  uint64_t size=channels*frames*itemsize;
  if(frames<1 || channels<1)
    return AMBIX_ERR_SUCCESS;
  if(size<1)
    return AMBIX_ERR_UNKNOWN;

  if(size > ambix->adaptorbuffersize) {
    /* re-allocate memory! */
    void*newbuf=realloc(ambix->adaptorbuffer, size);
    if(newbuf) {
      ambix->adaptorbuffer=newbuf;
      ambix->adaptorbuffersize=size;
    } else {
      free(ambix->adaptorbuffer);
      ambix->adaptorbuffer=NULL;
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


#define _AMBIX_SPLITADAPTOR(type)                                       \
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
_AMBIX_SPLITADAPTOR(float64);
_AMBIX_SPLITADAPTOR(int32);
_AMBIX_SPLITADAPTOR(int16);



#define _AMBIX_SPLITADAPTOR_MATRIX(type, sumtype)                       \
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
        sumtype##_t sum=0.;                                             \
        for(inchan=0; inchan<rawambichannels; inchan++) {               \
          sum+=mtx[outchan][inchan] * src[inchan];                      \
        }                                                               \
        *dest_ambi++=(type##_t)sum;  /* FIXXXME: integer saturation */  \
      }                                                                 \
      for(inchan=rawambichannels; inchan<sourcechannels; inchan++)      \
        *dest_other++=src[inchan];                                      \
    }                                                                   \
    return AMBIX_ERR_SUCCESS;                                           \
  }

_AMBIX_SPLITADAPTOR_MATRIX(float32, float32);
_AMBIX_SPLITADAPTOR_MATRIX(float64, float64);
/* both _int16 and _int32 are highly unoptimized!
 * LATER: add some fixed point magic to speed things up
 */
_AMBIX_SPLITADAPTOR_MATRIX(int32, float32);
_AMBIX_SPLITADAPTOR_MATRIX(int16, float32);

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
_AMBIX_MERGEADAPTOR(float64);
_AMBIX_MERGEADAPTOR(int32);
_AMBIX_MERGEADAPTOR(int16);


//#define _AMBIX_MERGEADAPTOR_MATRIX(type)      \

#define _AMBIX_MERGEADAPTOR_MATRIX(type, sumtype)                       \
  ambix_err_t _ambix_mergeAdaptormatrix_##type(const type##_t*ambi_data, const ambix_matrix_t*matrix, \
                                               const type##_t*otherdata, uint32_t source2channels, \
                                               type##_t*destination, int64_t frames) { \
    float32_t**mtx=matrix->data;                                        \
    const uint32_t fullambichannels=matrix->cols;                       \
    const uint32_t ambixchannels=matrix->rows;                          \
    int64_t f;                                                          \
    for(f=0; f<frames; f++) {                                           \
      /* encode ambisonics->ambix and store in destination */           \
      uint32_t outchan, inchan;                                         \
      const type##_t*src = ambi_data+fullambichannels*f;                \
      for(outchan=0; outchan<ambixchannels; outchan++) {                \
        sumtype##_t sum=0.;                                               \
        for(inchan=0; inchan<fullambichannels; inchan++) {              \
          sum+=mtx[outchan][inchan] * src[inchan];                      \
        }                                                               \
        *destination++=(type##_t)sum;                                   \
      }                                                                 \
      /* store the otherchannels */                                     \
      for(inchan=0; inchan<source2channels; inchan++)                   \
        *destination++=*otherdata++;                                    \
    }                                                                   \
    return AMBIX_ERR_SUCCESS;                                           \
  }

_AMBIX_MERGEADAPTOR_MATRIX(float32, float32);
_AMBIX_MERGEADAPTOR_MATRIX(float64, float64);
_AMBIX_MERGEADAPTOR_MATRIX(int32, float32);
_AMBIX_MERGEADAPTOR_MATRIX(int16, float32);
