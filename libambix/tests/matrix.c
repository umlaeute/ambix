/* matrix - test matrix functionality

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

#include "common.h"
#include <string.h>

static float32_t leftdata_4_3[]= {
   0.19, 0.06, 0.14,
   0.05, 0.08, 0.44,
   0.25, 0.9, 0.77,
   0.83, 0.51, 0.58,
};


static float32_t rightdata_3_2[]= {
   0.22, 0.46,
   0.36, 0.53,
   0.77, 0.85,
};

static float32_t resultdata_4_2[]= {
   0.1712, 0.2382,
   0.3786, 0.4394,
   0.9719, 1.2465,
   0.8128, 1.1451,
};

void mtxmul_tests(float32_t eps) {
  float32_t errf;
  ambix_matrix_t *left, *right, *result, *testresult;
  STARTTEST("");

 /* fill in some test data */
  left=ambix_matrix_init(4, 3, NULL);
  ambix_matrix_fill_data(left, leftdata_4_3);
  right=ambix_matrix_init(3, 2, NULL);
  ambix_matrix_fill_data(right, rightdata_3_2);
  testresult=ambix_matrix_init(4, 2, NULL);
  ambix_matrix_fill_data(testresult, resultdata_4_2);

  errf=matrix_diff(__LINE__, left, left, eps);
  fail_if(!(errf<eps), __LINE__, "diffing matrix with itself returned %f (>%f)", errf, eps);

  /* do some matrix multiplication */
  result=ambix_matrix_multiply(left, right, NULL);
  fail_if((NULL==result), __LINE__, "multiply into NULL did not create matrix");

  fail_if((result!=ambix_matrix_multiply(left, right, result)), __LINE__, "multiply into existing matrix returned new matrix");

#if 0
  matrix_print(left);
  matrix_print(right);
  matrix_print(result);
  printf("------------\n");
#endif
  errf=matrix_diff(__LINE__, testresult, result, eps);
  fail_if((errf>eps), __LINE__, "diffing two results of same multiplication returned %f (>%f)", errf, eps);

  ambix_matrix_destroy(left);
  ambix_matrix_destroy(right);
  ambix_matrix_destroy(result);
  ambix_matrix_destroy(testresult);
}
void mtxmul_eye_tests(float32_t eps) {
  float32_t errf;
  ambix_matrix_t *left, *result, *eye;
  STARTTEST("");
  eye=ambix_matrix_init(4, 4, NULL);
  fail_if((eye!=ambix_matrix_fill(eye, AMBIX_MATRIX_IDENTITY)), __LINE__, "filling unity matrix %p did not return original matrix %p", eye);

  left=ambix_matrix_init(4, 2, NULL);
  fail_if(AMBIX_ERR_SUCCESS!=ambix_matrix_fill_data(left, resultdata_4_2), __LINE__,
          "filling left data failed");

  result=ambix_matrix_init(4, 2, NULL);
  fail_if(AMBIX_ERR_SUCCESS!=ambix_matrix_fill_data(result, resultdata_4_2), __LINE__,
          "filling result data failed");

  fail_if((result!=ambix_matrix_multiply(eye, left, result)), __LINE__, "multiplication into matrix did not return original matrix");
#if 0
  matrix_print(eye);
  matrix_print(result);
  matrix_print(left);
#endif
  errf=matrix_diff(__LINE__, left, result, eps);
  fail_if((errf>eps), __LINE__, "diffing matrix M with E*M returned %f (>%f)", errf, eps);

  ambix_matrix_destroy(left);
  ambix_matrix_destroy(result);
  ambix_matrix_destroy(eye);
}
void datamul_tests(float32_t eps) {
  float32_t errf;
  float32_t*resultdata  = (float32_t*)calloc(2*4, sizeof(float32_t));
  float32_t*resultdataT = (float32_t*)calloc(4*2, sizeof(float32_t));
  float32_t*inputdata   = (float32_t*)calloc(2*3, sizeof(float32_t));

  ambix_matrix_t*mtx=NULL;
  STARTTEST("");

  mtx=ambix_matrix_init(4, 3, NULL);
  ambix_matrix_fill_data(mtx, leftdata_4_3);

  data_transpose(inputdata, rightdata_3_2, 3, 2);

  fail_if(AMBIX_ERR_SUCCESS!=ambix_matrix_multiply_float32(resultdata, mtx, inputdata, 2), __LINE__,
          "data multiplication failed");

  data_transpose(resultdataT, resultdata, 2, 4);

  errf=data_diff(__LINE__, resultdataT, resultdata_4_2, 4*2, eps);
  if(errf>eps) {
    printf("matrix:\n");
    matrix_print(mtx);
    printf("input:\n");
    data_print(inputdata, 3*2);

    printf("expected:\n");
    data_print(resultdata_4_2, 4*2);
    printf("calculated:\n");
    data_print(resultdataT   , 4*2);

  }
  fail_if(!(errf<eps), __LINE__, "diffing data multiplication returned %f (>%f)", errf, eps);

#if 0
  printf("matrix:\n");matrix_print(mtx);
  printf("input :\n");  data_print(rightdata_3_2, 3*2);
  printf("output:\n");  data_print(resultdata, 4*2);

  printf("target:\n");  data_print(resultdata_4_2, 4*2);
#endif


  if(mtx)ambix_matrix_destroy(mtx);
  free(resultdata);
  free(resultdataT);
  free(inputdata);
}

void datamul_eye_tests(float32_t eps) {
  float32_t errf;
  uint64_t frames=4096;
  uint32_t channels=16;
  float32_t*inputdata;
  float32_t*outputdata;
  float32_t freq=500;
  ambix_matrix_t eye = {0, 0, NULL};
  STARTTEST("");


  inputdata =data_sine(frames, channels, freq);
  outputdata=(float32_t*)malloc(sizeof(float32_t)*frames*channels);
  ambix_matrix_init(channels, channels, &eye);
  ambix_matrix_fill(&eye, AMBIX_MATRIX_IDENTITY);

  fail_if(AMBIX_ERR_SUCCESS!=ambix_matrix_multiply_float32(outputdata, &eye, inputdata, frames),
          __LINE__, "data multilplication failed");

  errf=data_diff(__LINE__, inputdata, outputdata, frames*channels, eps);
  fail_if(!(errf<eps), __LINE__, "diffing data multiplication returned %f (>%f)", errf, eps);

#if 0
  printf("matrix:\n");  matrix_print(&eye);
  printf("input :\n");  data_print(inputdata, frames*channels);
  printf("output:\n");  data_print(outputdata,frames*channels);
#endif

  free(inputdata);
  free(outputdata);
  ambix_matrix_deinit(&eye);
}

void datamul_4_2_tests(uint32_t chunksize, float32_t eps) {
  uint32_t r, c, rows, cols;
  float32_t errf;
  uint64_t frames=8;
  uint32_t rawchannels=2;
  uint32_t cokchannels=4;

  float32_t*inputdata;
  float32_t*outputdata;
  float32_t*targetdata;

  float32_t freq=500;

  ambix_matrix_t eye = {0, 0, NULL};
  STARTTEST("");



  inputdata =data_sine(frames, rawchannels, freq);
  targetdata=data_sine(frames, cokchannels, freq);

  outputdata=(float32_t*)malloc(sizeof(float32_t)*frames*cokchannels);

  ambix_matrix_init(cokchannels, rawchannels, &eye);
  rows=eye.rows;
  cols=eye.cols;

  for(r=0; r<rows; r++) {
    for(c=0; c<cols; c++) {
      eye.data[r][c]=(1+r+c)%2;
    }
  }

#if 0
  matrix_print(&eye);
  printf("input\n");
  data_print(inputdata, rawchannels*frames);
#endif

  fail_if(AMBIX_ERR_SUCCESS!=ambix_matrix_multiply_float32(outputdata, &eye, inputdata, frames),
          __LINE__, "data multilplication failed");

#if 0
  printf("output\n");
  data_print(outputdata, cokchannels*frames);

  printf("target\n");
  data_print(targetdata, cokchannels*frames);
#endif



  errf=data_diff(__LINE__, targetdata, outputdata, frames*cokchannels, eps);
  fail_if(!(errf<eps), __LINE__, "diffing data multiplication returned %f (>%f)", errf, eps);

#if 0
  printf("matrix:\n");  matrix_print(&eye);
  printf("input :\n");  data_print(inputdata, frames*channels);
  printf("output:\n");  data_print(outputdata,frames*channels);
#endif

  ambix_matrix_deinit(&eye);

  free(inputdata);
  free(outputdata);
  free(targetdata);
}







void create_tests(float32_t eps) {
  int rows=4;
  int cols=3;
  int cols2=2;
  ambix_matrix_t matrix, *left, *right;
  STARTTEST("");

  memset(&matrix, 0, sizeof(matrix));

  left=ambix_matrix_create();
  fail_if((left==NULL), __LINE__, "failed to create left matrix");
  fail_if((left->rows || left->cols), __LINE__, "created empty matrix has non-zero size");
  fail_if((left!=ambix_matrix_init(rows, cols, left)), __LINE__, "initializing existing matrix* returned new matrix");
  fail_if((left->rows!=rows || left->cols!=cols), __LINE__, "created matrix [%dx%d] does not match [%dx%d]", left->rows, left->cols, cols, cols2);

  right=ambix_matrix_init(cols, cols2, NULL);
  fail_if((right==NULL), __LINE__, "failed to create right matrix");
  fail_if((right->rows!=cols || right->cols!=cols2), __LINE__, "created matrix [%dx%d] does not match [%dx%d]", right->rows, right->cols, cols, cols2);

  fail_if((&matrix!=ambix_matrix_init(rows, cols2, &matrix)), __LINE__, "initializing existing matrix returned new matrix");
  fail_if((matrix.rows!=rows || matrix.cols!=cols2), __LINE__, "initialized matrix [%dx%d] does not match [%dx%d]", matrix.rows, matrix.cols, rows, cols2);


  ambix_matrix_deinit(&matrix);
  fail_if((matrix.rows || matrix.cols), __LINE__, "deinitialized matrix is non-zero");

  ambix_matrix_deinit(left);
  fail_if((left->rows || left->cols), __LINE__, "deinitialized matrix is non-zero");

  ambix_matrix_destroy(left);
  ambix_matrix_destroy(right);
}



int main(int argc, char**argv) {
#if 1
  create_tests(1e-7);
  mtxmul_tests(1e-7);
  mtxmul_eye_tests(1e-7);
  datamul_tests(1e-7);
  datamul_eye_tests(1e-7);
#endif
  datamul_4_2_tests(1024, 1e-7);

  pass();
  return 0;
}
