/* matrices - test preset matrices

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

#include "common.h"
#include <string.h>
#include <math.h>

float32_t matrix_check_diff(const char*name, ambix_matrix_t*mtx1,ambix_matrix_t*mtx2) {
  uint32_t col, row;
  float32_t errf=0.;
  fail_if((mtx1->cols != mtx2->cols), __LINE__, "matrix columns (%s) do not match %d!=%d", name, mtx1->cols, mtx2->cols);
  fail_if((mtx1->rows != mtx2->rows), __LINE__, "matrix rows (%s) do not match %d!=%d", name, mtx1->rows, mtx2->rows);

  for(col=0; col<mtx1->cols; col++) {
    for(row=0; row<mtx1->rows; row++) {
      errf+=fabs(mtx1->data[row][col] - mtx2->data[row][col]);
    }
  }

  return errf;
}



ambix_matrix_t*inverse_matrices(ambix_matrixtype_t typ, uint32_t rows, uint32_t cols) {
  ambix_matrixtype_t pyt = (ambix_matrixtype_t)(AMBIX_MATRIX_TO_AMBIX | typ);
  ambix_matrix_t*mtx=ambix_matrix_init(rows, cols, NULL);
  ambix_matrix_t*xtm=ambix_matrix_init(cols, rows, NULL);

  ambix_matrix_t*a2f=NULL;
  ambix_matrix_t*f2a=NULL;

  skip_if(NULL==mtx, __LINE__, "couldn't create mtx-matrix");
  skip_if(NULL==xtm, __LINE__, "couldn't create xtm-matrix");

  a2f=ambix_matrix_fill(mtx, typ);
  f2a=ambix_matrix_fill(xtm,  pyt);

  skip_if(NULL==a2f, __LINE__, "couldn't create a2f-matrix");
  skip_if(NULL==f2a, __LINE__, "couldn't create f2a-matrix");

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
  float32_t errf;
  float32_t eps=1e-6;

  STARTTEST("%s\n", name);

  result=inverse_matrices(typ, rows, cols);
  eye=ambix_matrix_init(result->rows, result->cols, eye);
  skip_if(NULL==eye, __LINE__, "couldn't create eye-matrix");
  eye=ambix_matrix_fill(eye, AMBIX_MATRIX_IDENTITY);
  skip_if(NULL==eye, __LINE__, "couldn't fill eye-matrix");

  errf=matrix_check_diff(name, result, eye);

  if(!(errf<eps)){
    matrix_print(result);
  }

  fail_if(!(errf<eps), __LINE__, "diffing matrices (%s) returned %g-%g=%g", name, errf, eps, errf-eps);

  ambix_matrix_destroy(result);
  ambix_matrix_destroy(eye);
}

void check_matrix(const char*name, ambix_matrixtype_t typ, uint32_t rows, uint32_t cols) {
  ambix_matrix_t*mtx=NULL;
  ambix_matrix_t*result=NULL;
  ambix_matrix_t*zeros=NULL;
  float32_t errf=0.f;
  float32_t eps=1e-20;

  STARTTEST("%s\n", name);

  mtx=ambix_matrix_init(rows, cols, mtx);
  skip_if(NULL==mtx, __LINE__, "couldn't create mtx-matrix");
  result=ambix_matrix_fill(mtx, typ);
//  skip_if(NULL==result, __LINE__, "couldn't fill result-matrix");

  fail_if((result==NULL), __LINE__, "matrix_fill returned NULL");
  fail_if((result!=mtx ), __LINE__, "matrix_fill did not return matrix %p (got %p)", mtx, result);

  zeros=ambix_matrix_init(result->rows, result->cols, zeros);
  skip_if(NULL==zeros, __LINE__, "couldn't create zeros-matrix");
  zeros=ambix_matrix_fill(zeros, AMBIX_MATRIX_ZERO);
  skip_if(NULL==zeros, __LINE__, "couldn't fill zeros-matrix");

  errf=matrix_check_diff(name, result, zeros);

  if(AMBIX_MATRIX_ZERO==typ) {
    fail_if(!(errf<eps), __LINE__, "zero matrix non-zero (%f>%f)", errf, eps);
  } else {
    fail_if((errf<eps), __LINE__, "non-zero matrix zero (%f<%f)", errf, eps);
  }


  ambix_matrix_destroy(mtx);
  ambix_matrix_destroy(zeros);
}
void check_nomatrix(const char*name, ambix_matrixtype_t typ, uint32_t rows, uint32_t cols) {
  ambix_matrix_t*mtx=NULL;
  ambix_matrix_t*result=NULL;
  float32_t errf=0.f;
  float32_t eps=1e-20;

  STARTTEST("%s\n", name);

  mtx=ambix_matrix_init(rows, cols, mtx);
  skip_if(NULL==mtx, __LINE__, "couldn't create mtx-matrix");
  result=ambix_matrix_fill(mtx, typ);
  fail_if(NULL!=result, __LINE__, "filled illegal result-matrix");
  ambix_matrix_destroy(mtx);
}


int main(int argc, char**argv) {
  uint32_t r, c, o;
  char name[64];
  name[63]=0;
  for(r=1; r<16; r++) {
    for(c=1; c<16; c++) {
      snprintf(name, 63, "zero[%d, %d]", r, c);
      check_matrix(name, AMBIX_MATRIX_ZERO, r, c);
      snprintf(name, 63, "one[%d, %d]", r, c);
      check_matrix(name, AMBIX_MATRIX_ONE, r, c);
      snprintf(name, 63, "identity[%d, %d]", r, c);
      check_matrix(name, AMBIX_MATRIX_IDENTITY, r, c);
    }
  }

  check_matrix("FuMa[ 1, 1]", AMBIX_MATRIX_FUMA,  1,  1);
  check_matrix("FuMa[ 4, 3]", AMBIX_MATRIX_FUMA,  4,  3);
  check_matrix("FuMa[ 4, 4]", AMBIX_MATRIX_FUMA,  4,  4);
  check_matrix("FuMa[ 9, 5]", AMBIX_MATRIX_FUMA,  9,  5);
  check_matrix("FuMa[ 9, 6]", AMBIX_MATRIX_FUMA,  9,  6);
  check_matrix("FuMa[ 9, 9]", AMBIX_MATRIX_FUMA,  9,  9);
  check_matrix("FuMa[16, 7]", AMBIX_MATRIX_FUMA, 16,  7);
  check_matrix("FuMa[16, 8]", AMBIX_MATRIX_FUMA, 16,  8);
  check_matrix("FuMa[16,11]", AMBIX_MATRIX_FUMA, 16, 11);
  check_matrix("FuMa[16,16]", AMBIX_MATRIX_FUMA, 16, 16);
  check_nomatrix("FuMa[ 4, 2]", AMBIX_MATRIX_FUMA,  4,  2);
  check_nomatrix("FuMa[16,10]", AMBIX_MATRIX_FUMA, 16, 10);
  check_nomatrix("FuMa[16,12]", AMBIX_MATRIX_FUMA, 16, 12);
  check_nomatrix("FuMa[16,13]", AMBIX_MATRIX_FUMA, 16, 13);
  check_nomatrix("FuMa[16,14]", AMBIX_MATRIX_FUMA, 16, 14);
  check_nomatrix("FuMa[16,15]", AMBIX_MATRIX_FUMA, 16, 15);
  check_nomatrix("FuMa[25,25]", AMBIX_MATRIX_FUMA, 25, 25);

  check_nomatrix("MaFu[ 4, 2]", AMBIX_MATRIX_TO_FUMA,  2,  4);
  check_nomatrix("MaFu[16,10]", AMBIX_MATRIX_TO_FUMA, 10, 16);
  check_nomatrix("MaFu[16,12]", AMBIX_MATRIX_TO_FUMA, 12, 16);
  check_nomatrix("MaFu[16,13]", AMBIX_MATRIX_TO_FUMA, 13, 16);
  check_nomatrix("MaFu[16,14]", AMBIX_MATRIX_TO_FUMA, 14, 16);
  check_nomatrix("MaFu[16,15]", AMBIX_MATRIX_TO_FUMA, 15, 16);
  check_nomatrix("MaFu[25,25]", AMBIX_MATRIX_TO_FUMA, 25, 25);

  check_nomatrix("sid2acn [ 7, 7]", AMBIX_MATRIX_SID,  7,  7);
  check_nomatrix("acn2sid [ 7, 7]", AMBIX_MATRIX_TO_SID,  7,  7);
  check_nomatrix("n3d->sn3d [ 7, 7]", AMBIX_MATRIX_N3D,  7,  7);
  check_nomatrix("sn3d->n3d [ 7, 7]", AMBIX_MATRIX_TO_N3D,  7,  7);
  check_nomatrix("nada[ 4, 4]", AMBIX_MATRIX_INVALID,  4,  4);

  STOPTEST("matrices\n");

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
  STOPTEST("matrix inversion\n");

  for(o=1; o<6; o++) {
    uint32_t chan=ambix_order2channels(o);
    snprintf(name, 63, "n3d2snd3d[%d, %d]", chan, chan);
    check_matrix(name, AMBIX_MATRIX_N3D, chan, chan);
    snprintf(name, 63, "sn3d2n3d[%d, %d]", chan, chan);
    check_matrix(name, AMBIX_MATRIX_TO_N3D, chan, chan);
    snprintf(name, 63, "n3d[%d, %d]", chan, chan);
    check_inversion(name, AMBIX_MATRIX_N3D,  1,  1);


    snprintf(name, 63, "sid2acn[%d, %d]", chan, chan);
    check_matrix(name, AMBIX_MATRIX_SID, chan, chan);
    snprintf(name, 63, "acn2sid[%d, %d]", chan, chan);
    check_matrix(name, AMBIX_MATRIX_TO_SID, chan, chan);

    snprintf(name, 63, "sid[%d, %d]", chan, chan);
    check_inversion(name, AMBIX_MATRIX_SID,  1,  1);
  }
  STOPTEST("\n");

  return pass();
}
