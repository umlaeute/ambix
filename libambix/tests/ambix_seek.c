#include "common.h"
#include "data.h"

int main()
{
  int result=0;
  
  ambix_t *ambix = NULL;
  ambix_info_t *info= calloc(1, sizeof(ambix_info_t));

  ambix=ambix_open(AMBIXTEST_FILE1, AMBIX_READ, info);
  skip_if(NULL==ambix, __LINE__, "ambix_open(%s, AMBIX_READ, info)", AMBIXTEST_FILE1);
  
  result= ambix_seek (ambix, 100, SEEK_CUR);
  fail_if (result!=100, __LINE__, "File failed to seek correct data");
  
  ambix_close(ambix);
  
  free(info);
  return 0;
}
