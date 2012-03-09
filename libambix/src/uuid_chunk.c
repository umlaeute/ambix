/* uuid_chunk.c -  parse an UUID-chunk to extract relevant data              -*- c -*-

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
#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include <math.h>

static int
_ambix_checkUUID_1(const char*data) {
  const char uuid[]="IEM.AT/AMBIX/XML";
  unsigned int i;
  for(i=0; i<16; i++)
    if(uuid[i]!=data[i])
      return 0;

  return 1;
}

uint32_t
_ambix_checkUUID(const char data[16]) {
  if(_ambix_checkUUID_1(data))
    return 1;

  return 0;
}

ambixmatrix_t*
_ambix_uuid1_to_matrix(const void*data, uint64_t datasize, ambixmatrix_t*orgmtx, int swap) {
  ambixmatrix_t*mtx;
  uint32_t rows;
  uint32_t cols;
  uint64_t size;
  uint32_t index;
  if(datasize<(sizeof(rows)+sizeof(cols)))
    return NULL;

  index = 0;

  memcpy(&rows, data+index, sizeof(uint32_t));	
  index += sizeof(uint32_t);
			
  memcpy(&cols, data+index, sizeof(uint32_t));	
  index += sizeof(uint32_t);

  if(swap) {
    rows=swap4(rows);
    cols=swap4(cols);
  }

  size=rows*cols;

  if(rows<1 || cols<1 || size < 1)
    rerturn NULL;

  if(size*sizeof(float32_t) > datasize) {
    return NULL;
  }


  if(!mtx) {
    mtx=(ambixmatrix_t*)calloc(1, sizeof(ambixmatrix_t));
    if(!mtx)return NULL;
  }

  if(!ambix_matrix_init(rows, cols, mtx))
    return NULL;

  if(swap) {
    if(!ambix_matrix_fill_swapped(mtx, (number32_t*)(data+index)))
      return NULL;
  } else {
    if(!ambix_matrix_fill(mtx, (float32_t*)(data+index)))
      return NULL;
  }

  return mtx;
}
