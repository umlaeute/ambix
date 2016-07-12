/* const_matrix - test constness in matrix operations

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

static float32_t data_3_2[]= {
   0.22, 0.46,
   0.36, 0.53,
   0.77, 0.85,
};
static float32_t data_3_4[]= {
 0.320059925923633, -0.616572833442599, -0.758203952544301,  1.397070173352668,
-0.475395139478048, -2.112396458091558,  1.443108803981482, -0.198593134445739,
 0.349719602203337,  2.683861685335670, -0.150602340058839, -0.196372558639406,
};
static float32_t data_4_4[]= {
  0.7131185686247925, 0.1054799265939327, 0.1882023608287114, 0.1496964665104298,
  0.9035382689904633, 0.0958506183093942, 0.1490156537909140, 0.6730762573692578,
  0.7110257215280688, 0.4278857180785819, 0.5050723092090162, 0.2342525090113509,
  0.1917073427152419, 0.3837280931544647, 0.0397484032568303, 0.5895499716980565,
};
static float32_t data_4_3[]= {
   0.19, 0.06, 0.14,
   0.05, 0.08, 0.44,
   0.25, 0.90, 0.77,
   0.83, 0.51, 0.58,
};
static float32_t data_4_2[]= {
   0.1712, 0.2382,
   0.3786, 0.4394,
   0.9719, 1.2465,
   0.8128, 1.1451,
};


static void test__amfd(unsigned int rows, unsigned int cols,
                       const float32_t*orgdata, unsigned long frames,
                       uint32_t line, float32_t eps) {
  ambix_matrix_t*mtx=ambix_matrix_init(rows, cols, NULL);
  float32_t*data=calloc(frames, sizeof(*orgdata));
  float32_t errf=0;
  STARTTEST("[%dx%d]\n", rows, cols);
  /* make a copy of the org data, and verify it */
  memcpy(data, orgdata, frames*sizeof(*orgdata));
  errf = data_diff(line, FLOAT32, orgdata, data, frames, eps);
  fail_if(!(errf<eps), line, "orgdata differs from memcpy by %g (>%g)", errf, eps);

  /* fill in the data */
  ambix_matrix_fill_data(mtx, data);

  /* check if data has been modified */
  errf = data_diff(line, FLOAT32, orgdata, data, frames, eps);
  fail_if(!(errf<eps), line, "orgdata differs from filled data by %g (>%g)", errf, eps);

  free(data);
  ambix_matrix_destroy(mtx);
}
static void test_ambix_matrix_fill_data (float32_t eps) {
  /*
    ambix_err_t ambix_matrix_fill_data (ambix_matrix_t *mtx, const float32_t *data) ;
   */
  test__amfd(3, 2, data_3_2, sizeof(data_3_2)/sizeof(*data_3_2), __LINE__, eps);
  test__amfd(3, 4, data_3_4, sizeof(data_3_4)/sizeof(*data_3_4), __LINE__, eps);
  test__amfd(4, 2, data_4_2, sizeof(data_4_2)/sizeof(*data_4_2), __LINE__, eps);
  test__amfd(4, 3, data_4_3, sizeof(data_4_3)/sizeof(*data_4_3), __LINE__, eps);
  test__amfd(4, 4, data_4_4, sizeof(data_4_4)/sizeof(*data_4_4), __LINE__, eps);
  STOPTEST("ALL\n");
}

static void test__amc(unsigned int rows, unsigned int cols,
                       const float32_t*orgdata, unsigned long frames,
                       uint32_t line, float32_t eps) {
  ambix_matrix_t org_mtx;
  float32_t**org_vectors=calloc(rows, sizeof(*org_vectors));
  float32_t**org_data=calloc(rows, sizeof(*org_data));
  ambix_matrix_t*mtx=ambix_matrix_init(rows, cols, NULL);
  ambix_matrix_t*result=0;
  ambix_matrix_fill_data(mtx, orgdata);
  uint32_t r;

  STARTTEST("[%dx%d]\n", rows, cols);
  /* backup the matrix */
  memcpy(&org_mtx, mtx, sizeof(org_mtx));
  memcpy(org_vectors, mtx->data, rows*sizeof(*org_data));

  for(r=0; r<rows; r++) {
    org_data[r]=calloc(cols, sizeof(*org_data[r]));
    memcpy(org_data[r], mtx->data[r], cols*sizeof(*org_data[r]));
  }

  /* call something */
  result=ambix_matrix_copy(mtx, result);

  /* check whether the mtx has been untouched */
  fail_if(!(org_mtx.rows==mtx->rows && org_mtx.cols==mtx->cols), line,
          "original matrix [%dx%d] does not match copied-from matrix [%dx%d]",
          org_mtx.rows, org_mtx.cols, mtx->rows, mtx->cols);
  fail_if(!(org_mtx.data==mtx->data), line,
          "original matrix data [%p] does not match copied-from matrix data [%p]",
          org_mtx.data, mtx->data);
  for(r=0; r<org_mtx.rows; r++) {
    float32_t*org_row=org_mtx.data[r];
    float32_t*row    =mtx->data[r];
    float32_t errf=0;
    fail_if(!(org_row == row), line,
            "original matrix row[%d: %p] does not match copied-from matrix row[%d: %p]",
            r, org_row, r, row);
    /* compare the actual row-vector */
    errf=data_diff(line, FLOAT32, org_data[r], mtx->data[r], cols, eps);
    fail_if(!(errf<eps), line, "orgdata differs from copied-from data by %g (>%g)", errf, eps);
  }

  /* cleanup */
  ambix_matrix_destroy(mtx);
  ambix_matrix_destroy(result);
  free(org_vectors);
  for(r=0; r<rows; r++) {
    free(org_data[r]);
  }
  free(org_data);
}
static void test_ambix_matrix_copy (float32_t eps) {
  /*
    ambix_matrix_t *ambix_matrix_copy (const ambix_matrix_t *src, ambix_matrix_t *dest) ;
   */
  test__amc(3, 2, data_3_2, sizeof(data_3_2)/sizeof(*data_3_2), __LINE__, eps);
  test__amc(3, 4, data_3_4, sizeof(data_3_4)/sizeof(*data_3_4), __LINE__, eps);
  test__amc(4, 2, data_4_2, sizeof(data_4_2)/sizeof(*data_4_2), __LINE__, eps);
  test__amc(4, 3, data_4_3, sizeof(data_4_3)/sizeof(*data_4_3), __LINE__, eps);
  test__amc(4, 4, data_4_4, sizeof(data_4_4)/sizeof(*data_4_4), __LINE__, eps);
  STOPTEST("ALL\n");
}
static void test__amm (unsigned int rows1, unsigned int cols1, const float32_t*data1,
                       unsigned int rows2, unsigned int cols2, const float32_t*data2,
                        uint32_t line, float32_t eps) {
  ambix_matrix_t*A=0, *B=0, *result=0;
  ambix_matrix_t*A0=0, *B0=0;
  float32_t errf=0;
  STARTTEST("[%dx%d]*[%dx%d]\n", rows1, cols1, rows2, cols2);

  A=ambix_matrix_init(rows1, cols1, A); ambix_matrix_fill_data(A, data1);
  B=ambix_matrix_init(rows2, cols2, B); ambix_matrix_fill_data(B, data2);
  A0=ambix_matrix_copy(A, A0); B0=ambix_matrix_copy(B, B0);
  result=ambix_matrix_multiply(A, B, result);

  errf=matrix_diff(line, A, A0, eps);
  fail_if(!(errf<eps), line, "left-hand multiplication matrix has changed by %f (>%f)", errf, eps);
  errf=matrix_diff(line, B, B0, eps);
  fail_if(!(errf<eps), line, "right-hand multiplication matrix has changed by %f (>%f)", errf, eps);

  if(A     )ambix_matrix_destroy(A);
  if(A0    )ambix_matrix_destroy(A0);
  if(B     )ambix_matrix_destroy(B);
  if(B0    )ambix_matrix_destroy(B0);
  if(result)ambix_matrix_destroy(result);
}

static void test_ambix_matrix_multiply (float32_t eps) {
  /*
    ambix_matrix_t *ambix_matrix_multiply (const ambix_matrix_t *A, const ambix_matrix_t *B, ambix_matrix_t *result) ;
   */
  test__amm(3,4,data_3_4, 4,4,data_4_4, __LINE__, eps);
  test__amm(3,4,data_3_4, 4,3,data_4_3, __LINE__, eps);
  test__amm(3,4,data_3_4, 4,2,data_4_2, __LINE__, eps);
  test__amm(3,4,data_3_4, 3,2,data_3_4, __LINE__, eps);

  STOPTEST("ALL\n");
}
static void test__amp(unsigned int rows, unsigned int cols,
                      const float32_t*data,
                      uint32_t line, float32_t eps) {
  ambix_matrix_t*A=0, *A0=0, *result=0;
  float32_t errf=0;
  STARTTEST("[%dx%d]\n", rows, cols);

  A=ambix_matrix_init(rows, cols, A); ambix_matrix_fill_data(A, data);
  A0=ambix_matrix_copy(A, A0);

  result=ambix_matrix_pinv(A, result);

  errf=matrix_diff(line, A, A0, eps);
  fail_if(!(errf<eps), line, "pseudo-inverted matrix has changed by %f (>%f)", errf, eps);

  if(A     )ambix_matrix_destroy(A);
  if(A0    )ambix_matrix_destroy(A0);
  if(result)ambix_matrix_destroy(result);
}
static void test_ambix_matrix_pinv(float32_t eps) {
  /*
    ambix_matrix_t* ambix_matrix_pinv(const ambix_matrix_t*matrix, ambix_matrix_t*pinv) ;
   */
  test__amp(3, 2, data_3_2, __LINE__, eps);
  test__amp(3, 4, data_3_4, __LINE__, eps);
  test__amp(4, 2, data_4_2, __LINE__, eps);
  test__amp(4, 3, data_4_3, __LINE__, eps);
  test__amp(4, 4, data_4_4, __LINE__, eps);
}
static void test_ambix_matrix_multiply_float32(float32_t eps) {
  /*
    ambix_err_t ambix_matrix_multiply_float32(float32_t *dest, const ambix_matrix_t *mtx, const float32_t *source, int64_t frames) ;
   */
  STARTTEST("\n");
  STOPTEST("\n");
}
static void test_ambix_matrix_multiply_float64(float32_t eps) {
  /*
    ambix_err_t ambix_matrix_multiply_float64(float64_t *dest, const ambix_matrix_t *mtx, const float64_t *source, int64_t frames) ;
   */
  STARTTEST("\n");
  STOPTEST("\n");
}
static void test_ambix_matrix_multiply_int32(float32_t eps) {
  /*
    ambix_err_t ambix_matrix_multiply_int32(int32_t *dest, const ambix_matrix_t *mtx, const int32_t *source, int64_t frames) ;
   */
  STARTTEST("\n");
  STOPTEST("\n");
}
static void test_ambix_matrix_multiply_int16(float32_t eps) {
  /*
    ambix_err_t ambix_matrix_multiply_int16(int16_t *dest, const ambix_matrix_t *mtx, const int16_t *source, int64_t frames) ;
   */
  STARTTEST("\n");
  STOPTEST("\n");
}



int main(int argc, char**argv) {
  test_ambix_matrix_fill_data(1e-7);
  test_ambix_matrix_copy(1e-7);
  test_ambix_matrix_multiply(1e-7);
  test_ambix_matrix_pinv(1e-7);

#warning  test_ambix_matrix_multiply_float32(1e-7);
#warning  test_ambix_matrix_multiply_float64(1e-7);
#warning  test_ambix_matrix_multiply_int32(1e-7);
#warning  test_ambix_matrix_multiply_int16(1e-7);

  return pass();
}

#if 0
/* FIXXME other constness tests */
ambix_t *ambix_open (const char *path, const ambix_filemode_t mode, ambix_info_t *ambixinfo) ;
int64_t ambix_writef_int16 (ambix_t *ambix, const int16_t *ambidata, const int16_t *otherdata, int64_t frames) ;
int64_t ambix_writef_int32 (ambix_t *ambix, const int32_t *ambidata, const int32_t *otherdata, int64_t frames) ;
int64_t ambix_writef_float32 (ambix_t *ambix, const float32_t *ambidata, const float32_t *otherdata, int64_t frames) ;
int64_t ambix_writef_float64 (ambix_t *ambix, const float64_t *ambidata, const float64_t *otherdata, int64_t frames) ;
ambix_err_t ambix_set_adaptormatrix (ambix_t *ambix, const ambix_matrix_t *matrix) ;
/* need to test for LATER side-effects: e.g. ambix_set_adaptormatrix() might not do anything NOW, but sometimes later... */
#endif
