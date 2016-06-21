#include "common_basic2extended.h"

int test_defaultmatrix(const char*name, uint32_t rows, uint32_t cols, ambix_matrixtype_t mtyp,
		       uint32_t xtrachannels, uint32_t chunksize, float32_t eps, ambixtest_presentationformat_t fmt) {
  int result=0;
  ambix_matrix_t*mtx=0;
  STARTTEST("%s\n", name);
  mtx=ambix_matrix_init(rows,cols,mtx);
  if(!mtx)return 1;
  ambix_matrix_fill(mtx, mtyp);
  result=check_create_b2e("test2-b2e-float32.caf", AMBIX_SAMPLEFORMAT_FLOAT32,
			  mtx,xtrachannels,
			  chunksize, fmt, eps);
  ambix_matrix_destroy(mtx);
  return result;
}

int main(int argc, char**argv) {
  int err=0;
  err+=test_defaultmatrix("IDENTITY1024:float32", 4, 4, AMBIX_MATRIX_IDENTITY, 0, 1024, 0, FLOAT32);
  err+=test_defaultmatrix("IDENTITY0000:float32", 4, 4, AMBIX_MATRIX_IDENTITY, 0,    0, 0, FLOAT32);
  err+=test_defaultmatrix("IDENTITY1024:int32", 4, 4, AMBIX_MATRIX_IDENTITY, 0, 1024, 0, INT32);
  err+=test_defaultmatrix("IDENTITY0000:int32", 4, 4, AMBIX_MATRIX_IDENTITY, 0,    0, 0, INT32);
  err+=test_defaultmatrix("IDENTITY1024:int16", 4, 4, AMBIX_MATRIX_IDENTITY, 0, 1024, 0, INT16);
  err+=test_defaultmatrix("IDENTITY0000:int16", 4, 4, AMBIX_MATRIX_IDENTITY, 0,    0, 0, INT16);
  return pass();
}
