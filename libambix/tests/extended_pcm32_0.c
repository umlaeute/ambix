#include "common.h"
#include <unistd.h>
#include <string.h>


void check_create_extended(const char*path, ambix_sampleformat_t format, uint32_t chunksize, float32_t eps);

int main(int argc, char**argv) {
  check_create_extended(FILENAME_FILE,  AMBIX_SAMPLEFORMAT_PCM32, 0, 1e-5);

  return pass();
}
