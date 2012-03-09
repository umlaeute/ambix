/* matrix.c -  Matrix handling              -*- c -*-

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

void
ambix_matrix_deinit(ambixmatrix_t*mtx) {
  uint32_t r;
  for(r=0; r<mtx->rows; r++) {
    if(mtx->data[r])
      free(mtx->data[r]);
    mtx->data[r]=NULL;
  }
  free(mtx->data);
  mtx->data=NULL;
  mtx->rows=0;
  mtx->cols=0;
}
ambixmatrix_t*
ambix_matrix_init(uint32_t rows, uint32_t cols, ambixmatrix_t*orgmtx) {
  ambixmatrix_t*mtx=orgmtx;
  uint32_t r;
  if(rows<1 || cols<1)
    return NULL;

  if(!mtx) {
    mtx=(ambixmatrix_t*)calloc(1, sizeof(ambixmatrix_t));
    if(!mtx)
      return NULL;
    mtx->data=NULL;
  }
  ambix_matrix_deinit(mtx);

  mtx->rows=rows;
  mtx->cols=cols;
  mtx->data=(float32_t**)calloc(rows, sizeof(float32_t*));
  for(r=0; r<rows; r++) {
    mtx->data[r]=(float32_t*)calloc(cols, sizeof(float32_t));
  }
  return mtx;
}
int
ambix_matrix_fill(ambixmatrix_t*mtx, float32_t*data) {
  float32_t**matrix=mtx->data;
  uint32_t rows=mtx->rows;
  uint32_t cols=mtx->cols;
  uint32_t r;
  for(r=0; r<rows; r++) {
    uint32_t c;
    for(c=0; c<cols; c++) {
      matrix[r][c]=*data++;
    }
  }
  return AMBIX_ERR_SUCCESS;
}

int
ambix_matrix_fill_swapped(ambixmatrix_t*mtx, number32_t*data) {
  float32_t**matrix=mtx->data;
  uint32_t rows=mtx->rows;
  uint32_t cols=mtx->cols;
  uint32_t r;
  for(r=0; r<rows; r++) {
    uint32_t c;
    for(c=0; c<cols; c++) {
      number32_t v;
      number32_t d = *data++;
      v.i=swap4(d.i);
      matrix[r][c]=v.f;
    }
  }
  return AMBIX_ERR_SUCCESS;
}



ambixmatrix_t*
ambix_matrix_copy(const ambixmatrix_t*src, ambixmatrix_t*dest) {
  if(!src)
    return NULL;
  if(!dest)
    dest=(ambixmatrix_t*)calloc(1, sizeof(ambixmatrix_t));

  if((dest->rows != src->rows) || (dest->cols != src->cols))
    ambix_matrix_init(src->rows, src->cols, dest);

  do {
    uint32_t r, c;
    float32_t**s=src->data;
    float32_t**d=dest->data;
    for(r=0; r<src->rows; r++) {
      for(c=0; c<src->cols; c++) {
        d[r][c]=s[r][c];
      }
    }
  } while(0);

  return dest;
}
