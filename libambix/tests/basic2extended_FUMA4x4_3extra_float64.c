#include "common_basic2extended.h"

int test_defaultmatrix(const char*name, uint32_t rows, uint32_t cols, ambix_matrixtype_t mtyp,
		       uint32_t xtrachannels, uint32_t chunksize, float32_t eps) {
  int result=0;
  ambix_matrix_t*mtx=0;
  STARTTEST("%s\n", name);
  mtx=ambix_matrix_init(rows,cols,mtx);
  if(!mtx)return 1;
  ambix_matrix_fill(mtx, mtyp);
  result=check_create_b2e("test2-b2e-float32.caf", AMBIX_SAMPLEFORMAT_FLOAT64,
			  mtx,xtrachannels,
			  chunksize, FLOAT32, eps);
  ambix_matrix_destroy(mtx);
  return result;
}

int main(int argc, char**argv) {
  int err=0;
  err+=test_defaultmatrix("FuMa[f]" , 4, 4, AMBIX_MATRIX_FUMA    , 3, 1024, 6e-8);
  return pass();
}
