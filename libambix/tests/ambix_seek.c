#include "common.h"

int main()
{
  
  ambix_t *ambix = NULL;
  ambix_info_t *info= calloc(1, sizeof(ambix_info_t));
  ambix=ambix_open(AMBIXTEST_FILE1, AMBIX_READ, info);
  
  int result=0;

  result= ambix_seek (ambix, 100, SEEK_CUR);
  fail_if (result!=100, __LINE__, "File failed to seek correct data");
  
  ambix_close(ambix);
  
  return 0;
}