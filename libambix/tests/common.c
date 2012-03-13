/* common test functionality 

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

#include "common.h"

void matrix_print(const ambixmatrix_t*mtx) {
  printf("matrix [%dx%d]\n", mtx->rows, mtx->cols);
  if(mtx->data) {
    uint32_t c, r;
    for(r=0; r<mtx->rows; r++) {
      for(c=0; c<mtx->cols; c++)
        printf(" %05f", mtx->data[r][c]);
      printf("\n");
    }
  }
}

float32_t*data_sine(uint64_t frames, uint32_t channels, float32_t periods) {
  float32_t*data=calloc(frames*channels, sizeof(float32_t));
  float32_t*datap;
  int64_t frame;
  for(frame=0; frame<frames; frame++) {
    float32_t value=sinf((float32_t)frame*(float32_t)frames/periods);
    int32_t chan;
    for(chan=0; chan<channels; chan++)
      *datap++=value;
  }

  return data;
}



float32_t matrix_diff(uint32_t line, const ambixmatrix_t*A, const ambixmatrix_t*B) {
  uint32_t r, c;
  float32_t sum=0.;

  float32_t**a=NULL;
  float32_t**b=NULL;

  fail_if((NULL==A), line, "left-hand matrix of matrixdiff is NULL");
  fail_if((NULL==B), line, "right-hand matrix of matrixdiff is NULL");
  fail_if(((A->rows!=B->rows) || (A->cols!=B->cols)), line, "matrixdiff matrices do not match [%dx%d]!=[%dx%d]", A->rows, A->cols, B->rows, B->cols);
  
  a=A->data;
  b=B->data;
  for(r=0; r<A->rows; r++)
    for(c=0; c<B->cols; c++) {
      float32_t v=a[r][c]-b[r][c];
      if(v<0)
        sum-=v;
      else
        sum+=v;
    }
  return sum;
}


float32_t data_diff(uint32_t line, const float32_t*A, const float32_t*B, uint64_t frames) {
  uint64_t i;
  float32_t sum=0.;

  fail_if((NULL==A), line, "left-hand data of datadiff is NULL");
  fail_if((NULL==B), line, "right-hand data of datadiff is NULL");
  
  for(i=0; i<frames; i++) {
    float32_t v=(*A++)-(*B++);
    if(v<0)
      sum-=v;
    else
      sum+=v;
  }
  return sum;
}
