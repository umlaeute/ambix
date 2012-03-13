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
  if(matrix->data) {
    uint32_t c, r;
    for(r=0; r<mtx->rows; r++) {
      for(c=0; c<mtx->cols; c++)
        printf(" %05f", mtx->data[r][c]);
      printf("\n");
    }
  }
}

float32_t*data_sine(uint32_t frames, float32_t periods) {
  float32_t*data=calloc(frames, sizeof(float32_t));
  int32_t f;
  for(frame=0; frame<frames; frame++) {
    data[frame]=sinf((float32_t)frame*(float32_t)frames/periods);
  }

  return data;
}
