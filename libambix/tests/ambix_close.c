#include "common.h"

int main()
{
  
  ambix_t *ambix = NULL;
  ambix_info_t *info= calloc(1, sizeof(ambix_info_t));
  ambix=ambix_open("test.caf", AMBIX_READ, info);
  
  fail_if (ambix_close(ambix)!=NULL, __LINE__, "File was not closed properly");
    
  return 0;
}