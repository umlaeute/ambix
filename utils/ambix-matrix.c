/* ambix-matrix -  display ambix matrices              -*- c -*-

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
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "ambix/ambix.h"

#include <stdio.h>
#include <string.h>

typedef struct {
  ambix_matrixtype_t typ;
  const char*name;
} matrix_map_t;

static matrix_map_t matrixmap[] = {
  AMBIX_MATRIX_ZERO, "MATRIX_ZERO",
  AMBIX_MATRIX_ONE, "MATRIX_ONE",
  AMBIX_MATRIX_AMBIX, "MATRIX_AMBIX",
  AMBIX_MATRIX_N3D, "MATRIX_N3D",
  AMBIX_MATRIX_SID, "MATRIX_SID",
  AMBIX_MATRIX_FUMA, "MATRIX_FUMA",
  AMBIX_MATRIX_TO_AMBIX, "MATRIX_TO_AMBIX",
  AMBIX_MATRIX_TO_N3D, "MATRIX_TO_N3D",
  AMBIX_MATRIX_TO_SID, "MATRIX_TO_SID",
  AMBIX_MATRIX_TO_FUMA, "MATRIX_TO_FUMA",
};



static void printmatrix(const ambix_matrix_t*mtx) {
  printf("matrix 0x%X", mtx);
  if(mtx) {
    float32_t**data=mtx->data;
    uint32_t r, c;
    printf(" [%dx%d] = %p\n", mtx->rows, mtx->cols, mtx->data);
    for(r=0; r<mtx->rows; r++) {
      for(c=0; c<mtx->cols; c++) {
        printf("%08f ", data[r][c]);
      }
      printf("\n");
    }
  }
  printf("\n");
}

static void print_matrix(const char*name, ambix_matrixtype_t typ, uint32_t rows, uint32_t cols) {
  ambix_matrix_t*mtx=ambix_matrix_init(rows, cols, NULL);
  if(mtx) {
    ambix_matrix_t*result=ambix_matrix_fill(mtx, typ);
    if(result) {
      printf("%s\t%d\n", name, (int)typ);
      printmatrix(result);
      if(result!=mtx)
        ambix_matrix_destroy(result);
    } else {
      printf("couldn't create matrix [%dx%d] of type %s[%d]\n", rows, cols, name, typ);
    }

    ambix_matrix_destroy(mtx);
  }
}


int main(int argc, char**argv) {
  print_matrix("FuMa [ ]"  , AMBIX_MATRIX_FUMA,  1,  1);
  print_matrix("FuMa [h]"  , AMBIX_MATRIX_FUMA,  4,  3);
  print_matrix("FuMa [f]"  , AMBIX_MATRIX_FUMA,  4,  4);
  print_matrix("FuMa [hh]" , AMBIX_MATRIX_FUMA,  9,  5);
  print_matrix("FuMa [fh]" , AMBIX_MATRIX_FUMA,  9,  6);
  print_matrix("FuMa [ff]" , AMBIX_MATRIX_FUMA,  9,  9);
  print_matrix("FuMa [hhh]", AMBIX_MATRIX_FUMA, 16,  7);
  print_matrix("FuMa [fhh]", AMBIX_MATRIX_FUMA, 16,  8);
  print_matrix("FuMa [ffh]", AMBIX_MATRIX_FUMA, 16, 11);
  print_matrix("FuMa [fff]", AMBIX_MATRIX_FUMA, 16, 16);



  return 0;
}
