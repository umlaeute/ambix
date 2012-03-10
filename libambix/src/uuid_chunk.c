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

#ifdef HAVE_STRING_H
# include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */


static const char _ambix_uuid_v1[]="IEM.AT/AMBIX/XML"; /* uarg, this is not a UUID! */
const char* _ambix_getUUID(uint32_t version) {
  switch(version) {
  default:
    break;
  case 1:
    return _ambix_uuid_v1;
  }
  return NULL;
}

static int
_ambix_checkUUID_1(const char*data) {
  unsigned int i;
  const char*uuid=_ambix_getUUID(1);
  if(!uuid) return 0;
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
  ambixmatrix_t*mtx=orgmtx;
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
    goto cleanup;

  if(size*sizeof(float32_t) > datasize) {
    goto cleanup;
  }

  if(!mtx) {
    mtx=(ambixmatrix_t*)calloc(1, sizeof(ambixmatrix_t));
    if(!mtx)
      goto cleanup;
  }

  if(!ambix_matrix_init(rows, cols, mtx))
      goto cleanup;

  if(swap) {
    if(ambix_matrix_fill_swapped(mtx, (number32_t*)(data+index)) != AMBIX_ERR_SUCCESS)
      goto cleanup;
  } else {
    if(ambix_matrix_fill(mtx, (float32_t*)(data+index)) != AMBIX_ERR_SUCCESS)
      goto cleanup;
  }
  return mtx;

 cleanup:
  if(mtx && mtx!=orgmtx) {
    ambix_matrix_deinit(mtx);
    free(mtx);
    mtx=NULL;
  }
  return NULL;
}


uint64_t
_ambix_matrix_to_uuid1(const ambixmatrix_t*matrix, void*data, int swap) {
  const char*uuid=_ambix_getUUID(1);
  uint64_t index=0;
  uint64_t datasize=0;
  uint32_t rows=matrix->rows;
  uint32_t cols=matrix->cols;
  float32_t**mtx=matrix->data;

  if(!uuid)
    return 0;

  datasize+=16; /* reserved for the UUID */

  datasize+=sizeof(uint32_t); /* rows */
  datasize+=sizeof(uint32_t); /* cols */
  datasize+=(uint64_t)rows*(uint64_t)cols*sizeof(float32_t); /* data */


  if(data) {
    uint64_t i, r, c;
    uint32_t*swapdata;
    uint64_t elements=(uint64_t)rows*(uint64_t)cols;
    memcpy(data, uuid, 16);
    index+=16;

    swapdata=data+index;

    memcpy(data+index, &rows, sizeof(uint32_t));
    index+=sizeof(uint32_t);

    memcpy(data+index, &cols, sizeof(uint32_t));
    index+=sizeof(uint32_t);

    for(r=0; r<rows; r++) {
      memcpy(data+index, mtx[r], cols*sizeof(float32_t));
      index+=cols*sizeof(float32_t);
    }

    if(swap) {
      _ambix_swap4array(swapdata, elements+2);
    }
  }
  return datasize;
}
