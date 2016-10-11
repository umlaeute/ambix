#include "common_basic2extended.h"

#ifndef FMT
# define FMT FLOAT32
#endif
#ifndef EPS
# define EPS 0
#endif

int test_defaultmatrix(const char*name, uint32_t rows, uint32_t cols, ambix_matrixtype_t mtyp,
		       uint32_t xtrachannels, uint32_t chunksize, float32_t eps, ambixtest_presentationformat_t fmt) {
  int result=0;
  ambix_matrix_t*mtx=0;
  STARTTEST("%s\n", name);
  mtx=ambix_matrix_init(rows,cols,mtx);
  if(!mtx)return 1;
  ambix_matrix_fill(mtx, mtyp);
  result=check_create_b2e(FILENAME_FILE, AMBIX_SAMPLEFORMAT_FLOAT32,
			  mtx,xtrachannels,
			  chunksize, fmt, eps);
  ambix_matrix_destroy(mtx);
  return result;
}

int main(int argc, char**argv) {
  int err=0;
  err+=test_defaultmatrix("IDENTITY1024:" STRINGIFY(FMT), 4, 4, AMBIX_MATRIX_IDENTITY, 0, 1024, EPS, FMT);
  err+=test_defaultmatrix("IDENTITY0000:" STRINGIFY(FMT), 4, 4, AMBIX_MATRIX_IDENTITY, 0,    0, EPS, FMT);
  return pass();
}
