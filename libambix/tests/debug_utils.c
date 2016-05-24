#include "common.h"
#include "data.h"

/* libambix's private header */
#include "private.h"

int main(int argc, char**argv) {
  ambix_info_t*info=NULL;
  ambix_matrix_t*mtx=NULL;
  ambix_t*ambix=NULL;
  _ambix_print_info(info);
  _ambix_print_matrix(mtx);
  _ambix_print_ambix(ambix);

  info= calloc(1, sizeof(ambix_info_t));
  ambix=ambix_open(AMBIXTEST_FILE1, AMBIX_READ, info);
  fail_if(NULL==ambix, __LINE__, "File was not open");

  _ambix_print_info(info);
  _ambix_print_ambix(ambix);

  ambix_close(ambix);
  free(info);

  mtx=ambix_matrix_init(4, 5, mtx);
  _ambix_print_matrix(mtx);
  mtx=ambix_matrix_fill(mtx, AMBIX_MATRIX_ONE);
  _ambix_print_matrix(mtx);
  ambix_matrix_destroy(mtx);

  return pass();
}
