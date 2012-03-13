/* matrix - test matrix functionality

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

static float32_t leftdata_4_3[]= {
0.983520, 0.034607, 0.648430,
0.394529, 0.630990, 0.092989,
0.059999, 0.319706, 0.121544,
0.733692, 0.901404, 0.161981,
};


static float32_t rightdata_3_2[]= {
  0.33431,   0.11569,
  0.54961,   0.84047,
  0.44430,   0.19395,
};

static float32_t resultdata_4_2[]= {
  0.63592,   0.26863,
  0.52001,   0.59400,
  0.24978,   0.29922,
  0.81267,   0.87390,
};


static float32_t matrix_diff(ambixmatrix_t*A, ambixmatrix_t*B) {
  uint32_t r, c;
  float32_t sum=0.;

  float32_t**a=NULL;
  float32_t**b=NULL;

  fail_if((NULL==A), __LINE__, "left-hand matrix of matrixdiff is NULL");
  fail_if((NULL==B), __LINE__, "right-hand matrix of matrixdiff is NULL");
  fail_if(((A->rows!=B->rows) || (A->cols!=B->cols)), __LINE__, "matrixdiff matrices do not match [%dx%d]!=[%dx%d]", A->rows, A->cols, B->rows, B->cols);
  
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

void create_tests(void) {
  int rows=4;
  int cols=3;
  int cols2=2;
  ambixmatrix_t matrix, *left, *right, *result, *eye;
  float32_t eps=1e-10;
  float32_t errf;

  float32_t*resultdata;

  memset(&matrix, 0, sizeof(matrix));
  resultdata=calloc(4*2, sizeof(float32_t));
  
  left=ambix_matrix_create();
  fail_if((left==NULL), __LINE__, "failed to create matrix");
  fail_if((left->rows || left->cols), __LINE__, "created empty matrix has non-zero size");
  fail_if((left!=ambix_matrix_init(rows, cols, left)), __LINE__, "initializing existing matrix* returned new matrix");
  fail_if((left->rows!=rows || left->cols!=cols), __LINE__, "created matrix [%dx%d] does not match [%dx%d]", left->rows, left->cols, cols, cols2);

  right=ambix_matrix_init(cols, cols2, NULL);
  fail_if((right->rows!=cols || right->cols!=cols2), __LINE__, "created matrix [%dx%d] does not match [%dx%d]", right->rows, right->cols, cols, cols2);

  fail_if((&matrix!=ambix_matrix_init(rows, cols2, &matrix)), __LINE__, "initializing existing matrix returned new matrix");
  fail_if((matrix.rows!=rows || matrix.cols!=cols2), __LINE__, "initialized matrix [%dx%d] does not match [%dx%d]", matrix.rows, matrix.cols, rows, cols2);
  
  eye=ambix_matrix_init(rows, rows, NULL);
  fail_if((eye!=ambix_matrix_eye(eye)), __LINE__, "filling unity matrix %p did not return original matrix %p", eye);


  /* fill in some test data */
  ambix_matrix_fill(left, leftdata_4_3);
  ambix_matrix_fill(right, rightdata_3_2);

  /* do some matrix multiplication */
  result=ambix_matrix_multiply(left, right, NULL);
  fail_if((NULL==result), __LINE__, "multiply into NULL did not create matrix");

  fail_if((&matrix!=ambix_matrix_multiply(left, right, &matrix)), __LINE__, "multiply into existing matrix returned new matrix");

  errf=matrix_diff(left, left);
  fail_if((errf>eps), __LINE__, "diffing matrix with itself returned %f (>%f)", errf, eps);

  errf=matrix_diff(&matrix, result);
  fail_if((errf>eps), __LINE__, "diffing two results of same multiplication returned %f (>%f)", errf, eps);


  fail_if((left!=ambix_matrix_multiply(eye, result, left)), __LINE__, "multiplication into matrix did not return original matrix");
  errf=matrix_diff(left, result);
  fail_if((errf>eps), __LINE__, "diffing matrix M with E*M returned %f (>%f)", errf, eps);


  /* do some data multiplication */
  fail_if(AMBIX_ERR_SUCCESS!=ambix_matrix_multiply_float32(resultdata, left, rightdata_3_2, 2), __LINE__,
          "data multilplication failed");
  fail_if(AMBIX_ERR_SUCCESS!=ambix_matrix_fill(result, resultdata), __LINE__,
          "filling result data failed");

   errf=matrix_diff(&matrix, result);
  fail_if((errf>eps), __LINE__, "diffing matrix multiplication with data multiplication returned %f (>%f)", errf, eps);

  ambix_matrix_deinit(&matrix);
  fail_if((matrix.rows || matrix.cols), __LINE__, "deinitialized matrix is non-zero");

  ambix_matrix_deinit(result);
  fail_if((result->rows || result->cols), __LINE__, "deinitialized matrix is non-zero");

  ambix_matrix_destroy(left);
  ambix_matrix_destroy(right);
  ambix_matrix_destroy(result);

  free(resultdata);
}



int main(int argc, char**argv) {
  create_tests();

  pass();
  return 0;
}
