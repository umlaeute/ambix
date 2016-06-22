#include "common_basic2extended.h"

float32_t data_4_9[]={
  0.519497, 0.101224, 0.775246, 0.219242, 0.795973, 0.649863, 0.190978, 0.837028, 0.763130,
  0.165074, 0.276581, 0.220167, 0.383229, 0.937749, 0.381838, 0.025107, 0.846256, 0.773257,
  0.546205, 0.501742, 0.476078, 0.539815, 0.671716, 0.069030, 0.748010, 0.369414, 0.667491,
  0.192167, 0.936164, 0.792496, 0.447073, 0.689901, 0.618242, 0.769460, 0.815128, 0.466140,
};

int test_datamatrix(const char*name, uint32_t rows, uint32_t cols, float32_t*data,
		    uint32_t xtrachannels, uint32_t chunksize, float32_t eps) {
  int result=0;
  ambix_matrix_t*mtx=0;
  STARTTEST("%s\n", name);
  mtx=ambix_matrix_init(rows,cols,mtx);
  if(!mtx)return 1;
  ambix_matrix_fill_data(mtx, data);
  result=check_create_b2e(FILENAME_FILE, AMBIX_SAMPLEFORMAT_FLOAT32,
			  mtx,xtrachannels,
			  chunksize, FLOAT32, eps);
  ambix_matrix_destroy(mtx);
  return result;
}

int main(int argc, char**argv) {
  int err=0;
  err+=test_datamatrix   ("'rand'[4x9]", 4, 9, data_4_9          , 3, 1024, 5e-7);
  return pass();
}
