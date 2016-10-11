#include "common_basic2extended.h"

float32_t data_4_7[]={
0.588219, 0.758465, 0.240167, 0.827193, 0.899134, 0.121186, 0.974490,
0.079994, 0.828276, 0.223099, 0.525774, 0.895537, 0.884317, 0.434495,
0.426266, 0.302895, 0.341656, 0.325299, 0.474342, 0.677271, 0.185883,
0.183465, 0.654321, 0.243639, 0.779875, 0.250082, 0.939078, 0.449736,
};

int test_datamatrix(const char*name, uint32_t rows, uint32_t cols, float32_t*data,
		    uint32_t xtrachannels, uint32_t chunksize, float32_t eps) {
  int result=0;
  ambix_matrix_t*mtx=0;
  STARTTEST("%s\n", name);
  mtx=ambix_matrix_init(rows,cols,mtx);
  if(!mtx)return 1;
  ambix_matrix_fill_data(mtx, data);
  result=check_create_b2e(FILENAME_FILE, AMBIX_SAMPLEFORMAT_PCM16,
			  mtx,xtrachannels,
			  chunksize, FLOAT32, eps);
  ambix_matrix_destroy(mtx);
  return result;
}

int main(int argc, char**argv) {
  int err=0;
  err+=test_datamatrix   ("'rand'[4x7]", 4, 7, data_4_7          , 0, 1024, 8e-5);
  return pass();
}
