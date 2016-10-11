/* matrix - test matrix functionality

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
#include <stdlib.h>

static float32_t leftdata_4_3[]= {
  1.89999997615814208984e-01, 5.99999986588954925537e-02, 1.40000000596046447754e-01,
  5.00000007450580596924e-02, 7.99999982118606567383e-02, 4.39999997615814208984e-01,
  2.50000000000000000000e-01, 8.99999976158142089844e-01, 7.69999980926513671875e-01,
  8.29999983310699462891e-01, 5.09999990463256835938e-01, 5.79999983310699462891e-01,
};
static float32_t leftdata_4_4[]= {
  7.13118553161621093750e-01, 1.05479925870895385742e-01, 1.88202366232872009277e-01, 1.49696469306945800781e-01,
  9.03538286685943603516e-01, 9.58506166934967041016e-02, 1.49015650153160095215e-01, 6.73076272010803222656e-01,
  7.11025714874267578125e-01, 4.27885711193084716797e-01, 5.05072295665740966797e-01, 2.34252512454986572266e-01,
  1.91707342863082885742e-01, 3.83728086948394775391e-01, 3.97484041750431060791e-02, 5.89549958705902099609e-01,
};
static float32_t rightdata_3_2[]= {
   0.22, 0.46,
   0.36, 0.53,
   0.77, 0.85,
};
static float32_t resultdata_4_2[]= {
  /* leftdata[4,3] * rightdata[3,2] */
   0.1712, 0.2382,
   0.3786, 0.4394,
   0.9719, 1.2465,
   0.8128, 1.1451,
};
static float32_t resultpinv_4_3[] = {
  /* (leftdata[4,3])^-1 */
  3.20059925317764282227e-01, -6.16572856903076171875e-01, -7.58203923702239990234e-01, 1.39707016944885253906e+00,
  -4.75395143032073974609e-01, -2.11239647865295410156e+00, 1.44310879707336425781e+00, -1.98593139648437500000e-01,
  3.49719613790512084961e-01, 2.68386173248291015625e+00, -1.50602340698242187500e-01, -1.96372553706169128418e-01,
};
static float32_t resultpinv_4_4[] = {
  /* (leftdata[4,4])^-1 */
 3.26068806648254394531e+00, -7.50604689121246337891e-01, -1.02798342704772949219e+00, 4.37466591596603393555e-01,
 4.47858381271362304688e+00, -3.52734327316284179688e+00, -8.83177340030670166016e-01, 3.24082684516906738281e+00,
 -6.75182533264160156250e+00, 2.95947027206420898438e+00, 3.87480235099792480469e+00, -3.20398116111755371094e+00,
 -3.52011203765869140625e+00, 2.34043407440185546875e+00, 6.47875070571899414062e-01, -3.39426249265670776367e-01,
};
static void mtxinverse_test(const ambix_matrix_t *mtx, const ambix_matrix_t *result, float32_t eps) {
  ambix_matrix_t *pinv = 0;
  ambix_matrix_t *mul=0;
  ambix_matrix_t *eye=0;
  float32_t errf;
  int min_rowcol=(mtx->cols<mtx->rows)?mtx->cols:mtx->rows;

  fail_if((NULL==mtx), __LINE__, "cannot invert NULL-matrix");
  eye=ambix_matrix_init(min_rowcol, min_rowcol, eye);
  eye=ambix_matrix_fill(eye, AMBIX_MATRIX_IDENTITY);
  fail_if((NULL==eye), __LINE__, "cannot create eye-matrix for pinv-verification");

  pinv=ambix_matrix_pinv(mtx, pinv);
  if(NULL==pinv)matrix_print(mtx);
  fail_if((NULL==pinv), __LINE__, "could not invert matrix");

  if(mtx->cols < mtx->rows)
    mul=ambix_matrix_multiply(pinv, mtx, 0);
  else
    mul=ambix_matrix_multiply(mtx, pinv, 0);

#if 0
  matrix_print(mtx);
  matrix_print(pinv);
  matrix_print(mul);
  if(result)matrix_print(result);
  printf("------------\n");
#endif

  if(result) {
    errf=matrix_diff(__LINE__, pinv, result, eps);
    fail_if((errf>eps), __LINE__, "diffing (pseudo)inverse returned %g (>%g)", errf, eps);

    errf=matrix_diff(__LINE__, mul, eye, eps);
    fail_if((errf>eps), __LINE__, "diffing mtx*pinv(mtx) returned %g (>%g)", errf, eps);
  } else {
    errf=matrix_diff(__LINE__, mul, eye, eps);
    fail_if((!(isnan(errf) || isinf(errf) || (errf>eps))), __LINE__, "diffing invalid mtx*pinv(mtx) returned %g (!>%g)", errf, eps);
  }

  ambix_matrix_destroy(pinv);
  ambix_matrix_destroy(mul);
  ambix_matrix_destroy(eye);
}
void mtxinverse_tests(float32_t eps) {
  float32_t errf;
  ambix_matrix_t *mtx=0, *testresult=0;
  float32_t*transposedata = (float32_t*)calloc(3*4, sizeof(float32_t));

  STARTTEST("\n");

  /* fill in some test data 4x4 */
  STARTTEST("[4x4]\n");
  mtx=ambix_matrix_init(4, 4, mtx);
  ambix_matrix_fill_data(mtx, leftdata_4_4);
  testresult=ambix_matrix_init(4, 4, testresult);
  ambix_matrix_fill_data(testresult, resultpinv_4_4);
  mtxinverse_test(mtx, testresult, eps);

  /* fill in some test data 4x3 */
  STARTTEST("[4x3]\n");
  mtx=ambix_matrix_init(4, 3, mtx);
  ambix_matrix_fill_data(mtx, leftdata_4_3);
  testresult=ambix_matrix_init(3, 4, testresult);
  ambix_matrix_fill_data(testresult, resultpinv_4_3);
  mtxinverse_test(mtx, testresult, eps);

  /* fill in some test data 3x4 */
  STARTTEST("[3x4]\n");
  data_transpose(transposedata, leftdata_4_3, 4, 3);
  mtx=ambix_matrix_init(3, 4, mtx);
  ambix_matrix_fill_data(mtx, transposedata);

  data_transpose(transposedata, resultpinv_4_3, 3, 4);
  testresult=ambix_matrix_init(4, 3, testresult);
  ambix_matrix_fill_data(testresult, transposedata);
  mtxinverse_test(mtx, testresult, eps);

  /* fill in some test data 4x4 */
  STARTTEST("[identity:4x4]\n");
  mtx=ambix_matrix_init(4, 4, mtx);
  ambix_matrix_fill(mtx, AMBIX_MATRIX_IDENTITY);
  testresult=ambix_matrix_init(4, 4, testresult);
  ambix_matrix_fill(testresult, AMBIX_MATRIX_IDENTITY);
  mtxinverse_test(mtx, testresult, eps);

  STARTTEST("[one:4x4]\n");
  mtx=ambix_matrix_init(4, 4, mtx);
  ambix_matrix_fill(mtx, AMBIX_MATRIX_ONE);
  mtxinverse_test(mtx, NULL, eps);
  STARTTEST("[zero:4x4]\n");
  mtx=ambix_matrix_init(4, 4, mtx);
  ambix_matrix_fill(mtx, AMBIX_MATRIX_ZERO);
  mtxinverse_test(mtx, NULL, eps);

  STARTTEST("[SID:4x4]\n");
  mtx=ambix_matrix_init(4, 4, mtx);
  ambix_matrix_fill(mtx, AMBIX_MATRIX_SID);
  testresult=ambix_matrix_init(4, 4, testresult);
  ambix_matrix_fill(testresult, AMBIX_MATRIX_TO_SID);
  mtxinverse_test(mtx, testresult, eps);

  STARTTEST("[N3D:4x4]\n");
  mtx=ambix_matrix_init(4, 4, mtx);
  ambix_matrix_fill(mtx, AMBIX_MATRIX_N3D);
  testresult=ambix_matrix_init(4, 4, testresult);
  ambix_matrix_fill(testresult, AMBIX_MATRIX_TO_N3D);
  mtxinverse_test(mtx, testresult, eps);

  STARTTEST("[FUMA:4x4]\n");
  mtx=ambix_matrix_init(4, 4, mtx);
  ambix_matrix_fill(mtx, AMBIX_MATRIX_FUMA);
  testresult=ambix_matrix_init(4, 4, testresult);
  ambix_matrix_fill(testresult, AMBIX_MATRIX_TO_FUMA);
  mtxinverse_test(mtx, testresult, eps);


  ambix_matrix_destroy(mtx);
  ambix_matrix_destroy(testresult);
  free(transposedata);
  STOPTEST("\n");
}
void mtxmul_tests(float32_t eps) {
  float32_t errf;
  ambix_matrix_t *left=NULL, *right=NULL, *result, *testresult;
  STARTTEST("\n");

 /* fill in some test data */
  left=ambix_matrix_init(4, 3, NULL);
  ambix_matrix_fill_data(left, leftdata_4_3);
  right=ambix_matrix_init(3, 2, NULL);
  ambix_matrix_fill_data(right, rightdata_3_2);
  testresult=ambix_matrix_init(4, 2, NULL);
  ambix_matrix_fill_data(testresult, resultdata_4_2);

  errf=matrix_diff(__LINE__, left, left, eps);
  fail_if(!(errf<eps), __LINE__, "diffing matrix with itself returned %f (>%f)", errf, eps);

  /* NULL multiplications */
  result=ambix_matrix_multiply(NULL, NULL, NULL);
  fail_if(NULL!=result, __LINE__, "multiplying NULL*NULL returned success");
  result=ambix_matrix_multiply(left, NULL, result);
  fail_if(NULL!=result, __LINE__, "multiplying left*NULL returned success");
  result=ambix_matrix_multiply(NULL, left, result);
  fail_if(NULL!=result, __LINE__, "multiplying NULL*left returned success");

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
  STOPTEST("\n");
}
void mtxmul_eye_tests(float32_t eps) {
  float32_t errf;
  ambix_matrix_t *left, *result, *eye;
  STARTTEST("\n");
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
  STOPTEST("\n");
}
void datamul_tests(float32_t eps) {
  float32_t errf;
  float32_t*resultdata  = (float32_t*)calloc(2*4, sizeof(float32_t));
  float32_t*resultdataT = (float32_t*)calloc(4*2, sizeof(float32_t));
  float32_t*inputdata   = (float32_t*)calloc(2*3, sizeof(float32_t));

  fail_if((NULL==resultdata), __LINE__, "couldn't callocate resultdata");
  fail_if((NULL==resultdataT), __LINE__, "couldn't callocate resultdataT");
  fail_if((NULL==inputdata), __LINE__, "couldn't callocate inputdata");

  ambix_matrix_t*mtx=NULL;
  STARTTEST("\n");

  mtx=ambix_matrix_init(4, 3, NULL);
  ambix_matrix_fill_data(mtx, leftdata_4_3);

  data_transpose(inputdata, rightdata_3_2, 3, 2);

  fail_if(AMBIX_ERR_SUCCESS!=ambix_matrix_multiply_float32(resultdata, mtx, inputdata, 2), __LINE__,
          "data multiplication failed");

  data_transpose(resultdataT, resultdata, 2, 4);

  errf=data_diff(__LINE__, FLOAT32, resultdataT, resultdata_4_2, 4*2, eps);
  if(errf>eps) {
    printf("matrix:\n");
    matrix_print(mtx);
    printf("input:\n");
    data_print(FLOAT32, inputdata, 3*2);

    printf("expected:\n");
    data_print(FLOAT32, resultdata_4_2, 4*2);
    printf("calculated:\n");
    data_print(FLOAT32, resultdataT   , 4*2);

  }
  fail_if(!(errf<eps), __LINE__, "diffing data multiplication returned %f (>%f)", errf, eps);

#if 0
  printf("matrix:\n");matrix_print(mtx);
  printf("input :\n");  data_print(FLOAT32, rightdata_3_2, 3*2);
  printf("output:\n");  data_print(FLOAT32, resultdata, 4*2);

  printf("target:\n");  data_print(FLOAT32, resultdata_4_2, 4*2);
#endif


  if(mtx)ambix_matrix_destroy(mtx);
  free(resultdata);
  free(resultdataT);
  free(inputdata);
  STOPTEST("\n");
}

void datamul_eye_tests(float32_t eps) {
  float32_t errf;
  uint64_t frames=4096;
  uint32_t channels=16;
  float32_t*inputdata;
  float32_t*outputdata;
  float32_t freq=500;
  ambix_matrix_t eye = {0, 0, NULL};
  STARTTEST("\n");

  inputdata =data_sine(FLOAT32, frames, channels, freq);
  outputdata=(float32_t*)malloc(sizeof(float32_t)*frames*channels);
  fail_if((NULL==outputdata), __LINE__, "couldn't mallocate outputdata");

  ambix_matrix_init(channels, channels, &eye);
  ambix_matrix_fill(&eye, AMBIX_MATRIX_IDENTITY);

  fail_if(AMBIX_ERR_SUCCESS!=ambix_matrix_multiply_float32(outputdata, &eye, inputdata, frames),
          __LINE__, "data multilplication failed");

  errf=data_diff(__LINE__, FLOAT32, inputdata, outputdata, frames*channels, eps);
  fail_if(!(errf<eps), __LINE__, "diffing data multiplication returned %f (>%f)", errf, eps);

#if 0
  printf("matrix:\n");  matrix_print(&eye);
  printf("input :\n");  data_print(FLOAT32, inputdata, frames*channels);
  printf("output:\n");  data_print(FLOAT32, outputdata,frames*channels);
#endif

  free(inputdata);
  free(outputdata);
  ambix_matrix_deinit(&eye);
  STOPTEST("\n");
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
  STARTTEST("\n");

  inputdata =data_sine(FLOAT32, frames, rawchannels, freq);
  targetdata=data_sine(FLOAT32, frames, cokchannels, freq);

  outputdata=(float32_t*)malloc(sizeof(float32_t)*frames*cokchannels);
  fail_if((NULL==outputdata), __LINE__, "couldn't allocate outputdata");

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
  data_print(FLOAT32, inputdata, rawchannels*frames);
#endif

  fail_if(AMBIX_ERR_SUCCESS!=ambix_matrix_multiply_float32(outputdata, &eye, inputdata, frames),
          __LINE__, "data multilplication failed");

#if 0
  printf("output\n");
  data_print(FLOAT32, outputdata, cokchannels*frames);

  printf("target\n");
  data_print(FLOAT32, targetdata, cokchannels*frames);
#endif



  errf=data_diff(__LINE__, FLOAT32, targetdata, outputdata, frames*cokchannels, eps);
  fail_if(!(errf<eps), __LINE__, "diffing data multiplication returned %f (>%f)", errf, eps);

#if 0
  printf("matrix:\n");  matrix_print(&eye);
  printf("input :\n");  data_print(FLOAT32, inputdata, frames*channels);
  printf("output:\n");  data_print(FLOAT32, outputdata,frames*channels);
#endif

  ambix_matrix_deinit(&eye);

  free(inputdata);
  free(outputdata);
  free(targetdata);
  STOPTEST("\n");
}

void mtx_copy(float32_t eps) {
  float32_t errf;
  ambix_matrix_t *left=NULL, *right=NULL;
  unsigned int i;
  float32_t maxeps=eps;

  STARTTEST("\n");

  right=ambix_matrix_copy(left, NULL);
  fail_if((NULL!=right), __LINE__, "copying from NULL matrix erroneously succeeded");

  left=ambix_matrix_create();
  fail_if((left !=ambix_matrix_init(4, 3, left )), __LINE__, "initializing left matrix failed");
  ambix_matrix_fill_data(left, leftdata_4_3);

  right=ambix_matrix_copy(left, NULL);
  fail_if((NULL==right), __LINE__, "copying to NULL matrix failed");
  errf=matrix_diff(__LINE__, left, right, eps);
  fail_if(errf>0.f, __LINE__, "diffing mtx with copy0 returned %g (>%g)", errf, 0.f);

  right=ambix_matrix_copy(left, right);
  fail_if((NULL==right), __LINE__, "copying to right matrix failed");
  errf=matrix_diff(__LINE__, left, right, eps);
  fail_if(errf>0.f, __LINE__, "diffing mtx with copy returned %g (>%g)", errf, 0.f);

  ambix_matrix_destroy(left);
  ambix_matrix_destroy(right);
  STOPTEST("\n");

}

void mtx_diff(float32_t eps) {
  float32_t errf;
  ambix_matrix_t *left=NULL, *right=NULL;
  unsigned int i;
  const unsigned int rows=4;
  const unsigned int cols=3;
  float32_t*leftdata=leftdata_4_3;
  float32_t*rightdata=malloc(sizeof(leftdata_4_3));
  float32_t maxeps=eps;

  STARTTEST("\n");

  left=ambix_matrix_create();
  right=ambix_matrix_create();

  /* comparisons:
     - failing tests:
       - different dimensions
       - left/right matrix is NULL
     - non-failing tests:
       - all values diff==0
       - all values diff<eps
       - few values diff<eps
       - many values diff<eps
  */
  fail_if((left !=ambix_matrix_init(3, 4, left )), __LINE__, "initializing left matrix failed");
  fail_if((right!=ambix_matrix_init(3, 4, right)), __LINE__, "initializing right matrix failed");

  /* compare equal matrices */
  STARTTEST("ident\n");
  ambix_matrix_fill_data(left, leftdata);
  errf=matrix_diff(__LINE__, left, left, eps);
  fail_if(errf>0.f, __LINE__, "diffing mtx with itself returned %g (>%g)", errf, 0.f);

  /* compare equal matrices */
  STARTTEST("equal\n");
  for(i=0; i<rows*cols; i++) {
    rightdata[i]=leftdata[i];
  }
  ambix_matrix_fill_data(left , leftdata);
  ambix_matrix_fill_data(right, rightdata);
  errf=matrix_diff(__LINE__, left, right, eps);
  fail_if(errf>0.f, __LINE__, "diffing mtx with copy returned %g (>%g)", errf, 0.f);

  /* compare matrices where all values differ, but <eps */
  STARTTEST("all<eps\n");
  for(i=0; i<rows*cols; i++) {
    rightdata[i]=leftdata[i]+eps*0.5;
  }
  ambix_matrix_fill_data(left , leftdata);
  ambix_matrix_fill_data(right, rightdata);
  errf=matrix_diff(__LINE__, left, right, eps);
  fail_if(errf>eps, __LINE__, "diffing mtx with mtx+eps/2 returned %g (>%g)", errf, eps);
  for(i=0; i<rows*cols; i++) {
    rightdata[i]=leftdata[i]-eps*0.5;
  }
  ambix_matrix_fill_data(left , leftdata);
  ambix_matrix_fill_data(right, rightdata);
  errf=matrix_diff(__LINE__, left, right, eps);
  fail_if(errf>eps, __LINE__, "diffing mtx with mtx-eps/2 returned %g (>%g)", errf, eps);

  /* compare matrices where many values differ with <eps; but one with >eps */
  STARTTEST("most<eps;one>eps\n");
  for(i=0; i<rows*cols; i++) {
    rightdata[i]=leftdata[i];
  }
  for(i=0; i<rows; i++) {
    rightdata[i]=leftdata[i]+eps*0.5;
  }
  rightdata[0]=leftdata[0]+eps*1.5;
  ambix_matrix_fill_data(left , leftdata);
  ambix_matrix_fill_data(right, rightdata);
  errf=matrix_diff(__LINE__, left, right, eps);
  fail_if(errf>(eps*2.0), __LINE__, "diffing mtx with one value>eps returned %g (>%g)", errf, eps);
  fail_if(errf<(eps*1.0), __LINE__, "diffing mtx with one value>eps returned %g (>%g)", errf, eps);

  /* compare matrices where most values differ with >eps */
  STARTTEST("most>eps\n");
  for(i=0; i<rows*cols; i++) {
    rightdata[i]=leftdata[i];
  }
  maxeps=eps*1.5;
  for(i=0; i<(rows*cols)-1; i++) {
    rightdata[i]=leftdata[i]-maxeps;
  }
  ambix_matrix_fill_data(left , leftdata);
  ambix_matrix_fill_data(right, rightdata);
  errf=matrix_diff(__LINE__, left, right, eps);
  fail_if(errf<eps*1.0, __LINE__, "diffing mtx with one value>eps returned %g (<%g)", errf, eps*1.0);
  fail_if(errf>eps*2.0, __LINE__, "diffing mtx with one value>eps returned %g (<%g)", errf, eps*2.0);

  ambix_matrix_destroy(left);
  ambix_matrix_destroy(right);
  free(rightdata);
  STOPTEST("\n");
}

void create_tests(float32_t eps) {
  int rows=4;
  int cols=3;
  int cols2=2;
  ambix_matrix_t matrix, *left, *right;
  STARTTEST("\n");

  memset(&matrix, 0, sizeof(matrix));
  matrix_print(&matrix);

  left=ambix_matrix_create();
  fail_if((left==NULL), __LINE__, "failed to create left matrix");
  fail_if((left->rows || left->cols), __LINE__, "created empty matrix has non-zero size");
  fail_if((left!=ambix_matrix_init(rows, cols, left)), __LINE__, "initializing existing matrix* returned new matrix");
  fail_if((left->rows!=rows || left->cols!=cols), __LINE__, "created matrix [%dx%d] does not match [%dx%d]", left->rows, left->cols, cols, cols2);
  matrix_print(left);

  right=ambix_matrix_init(cols, cols2, NULL);
  fail_if((right==NULL), __LINE__, "failed to create right matrix");
  fail_if((right->rows!=cols || right->cols!=cols2), __LINE__, "created matrix [%dx%d] does not match [%dx%d]", right->rows, right->cols, cols, cols2);
  matrix_print(right);

  fail_if((&matrix!=ambix_matrix_init(rows, cols2, &matrix)), __LINE__, "initializing existing matrix returned new matrix");
  fail_if((matrix.rows!=rows || matrix.cols!=cols2), __LINE__, "initialized matrix [%dx%d] does not match [%dx%d]", matrix.rows, matrix.cols, rows, cols2);


  ambix_matrix_deinit(&matrix);
  fail_if((matrix.rows || matrix.cols), __LINE__, "deinitialized matrix is non-zero");

  ambix_matrix_deinit(left);
  fail_if((left->rows || left->cols), __LINE__, "deinitialized matrix is non-zero");

  ambix_matrix_destroy(left);
  ambix_matrix_destroy(right);
  STOPTEST("\n");
}



int main(int argc, char**argv) {
#if 1
  create_tests(1e-7);
  mtx_copy(1e-7);
  mtx_diff(1e-1);
  mtx_diff(1e-7);
  mtxmul_tests(1e-7);
  mtxmul_eye_tests(1e-7);
  datamul_tests(1e-7);
  datamul_eye_tests(1e-7);
#endif
  datamul_4_2_tests(1024, 1e-7);
  mtxinverse_tests(3e-5);

  return pass();
}
