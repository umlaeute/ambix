/* matrices - test preset matrices

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
#include <string.h>

void matrix_check_diff(const char*name, uint32_t line, ambix_matrix_t*mtx1,ambix_matrix_t*mtx2, float32_t eps) {
  uint32_t col, row;
  float32_t errf=0.;
  fail_if((mtx1->cols != mtx2->cols), line, "matrix columns (%s) do not match %d!=%d", name, mtx1->cols, mtx2->cols);
  fail_if((mtx1->rows != mtx2->rows), line, "matrix rows (%s) do not match %d!=%d", name, mtx1->rows, mtx2->rows);

  for(col=0; col<mtx1->cols; col++) {
    for(row=0; row<mtx1->rows; row++) {
      errf+=abs(mtx1->data[row][col] - mtx2->data[row][col]);
    }
  }

  if(!(errf<eps)){
    matrix_print(mtx1);
    matrix_print(mtx2);
  }

  fail_if(!(errf<eps), line, "diffing matrices (%s) returned %f (>%f)", name, errf, eps);
}



ambix_matrix_t*inverse_matrices(ambix_matrixtype_t typ, uint32_t rows, uint32_t cols) {
  ambix_matrix_t*mtx=ambix_matrix_init(rows, cols, NULL);
  ambix_matrix_t*xtm=ambix_matrix_init(cols, rows, NULL);

  ambix_matrix_t*a2f=ambix_matrix_fill(mtx, typ);
  ambix_matrix_t*f2a=ambix_matrix_fill(xtm,  AMBIX_MATRIX_TO_AMBIX | typ);

  //  ambix_matrix_t*result=ambix_matrix_multiply(a2f, f2a, NULL);
  ambix_matrix_t*result=ambix_matrix_multiply(f2a, a2f, NULL);

  if(a2f!=mtx)
    ambix_matrix_destroy(a2f);
  ambix_matrix_destroy(mtx);

  if(f2a!=xtm)
    ambix_matrix_destroy(f2a);
  ambix_matrix_destroy(xtm);

  return result;
}

void check_inversion(const char*name, ambix_matrixtype_t typ, uint32_t rows, uint32_t cols) {
  ambix_matrix_t*eye=NULL;
  ambix_matrix_t*result=NULL;

  result=inverse_matrices(typ, rows, cols);
  eye=ambix_matrix_init(result->rows, result->cols, eye);
  eye=ambix_matrix_fill(eye, AMBIX_MATRIX_IDENTITY);

  matrix_check_diff(name, __LINE__, result, eye, 1e-30);
}


int main(int argc, char**argv) {
  check_inversion("FuMa[ 1, 1]", AMBIX_MATRIX_FUMA,  1,  1);
  check_inversion("FuMa[ 4, 3]", AMBIX_MATRIX_FUMA,  4,  3);
  check_inversion("FuMa[ 4, 4]", AMBIX_MATRIX_FUMA,  4,  4);
  check_inversion("FuMa[ 9, 5]", AMBIX_MATRIX_FUMA,  9,  5);
  check_inversion("FuMa[ 9, 6]", AMBIX_MATRIX_FUMA,  9,  6);
  check_inversion("FuMa[ 9, 9]", AMBIX_MATRIX_FUMA,  9,  9);
  check_inversion("FuMa[16, 7]", AMBIX_MATRIX_FUMA, 16,  7);
  check_inversion("FuMa[16, 8]", AMBIX_MATRIX_FUMA, 16,  8);
  check_inversion("FuMa[16,11]", AMBIX_MATRIX_FUMA, 16, 11);
  check_inversion("FuMa[16,16]", AMBIX_MATRIX_FUMA, 16, 16);


  pass();
  return 0;
}
