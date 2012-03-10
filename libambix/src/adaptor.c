/* adaptor.c -  extracting ambisonics data from data using adaptor matrices              -*- c -*-

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
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */

ambix_err_t _ambix_adaptorbuffer_resize(ambix_t*ambix, uint64_t frames, uint16_t itemsize) {
  uint32_t channels=ambix->info.ambichannels + ambix->info.otherchannels;

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

  //    DEFAULT_ADAPTORBUFFER_SIZE
  return AMBIX_ERR_UNKNOWN;
}

ambix_err_t _ambix_adaptorbuffer_destroy(ambix_t*ambix) {
  if(ambix->adaptorbuffer)
    free(ambix->adaptorbuffer);
  ambix->adaptorbuffer=NULL;
  ambix->adaptorbuffersize=0;
  return AMBIX_ERR_SUCCESS;
}
