#include "common.h"
#include "data.h"

int main()
{
  
  ambix_t *ambix = NULL;
  ambix_info_t *info= calloc(1, sizeof(ambix_info_t));
  ambix=ambix_open(AMBIXTEST_FILE1, AMBIX_READ, info);
  skip_if (NULL==ambix, __LINE__, "ambix_open(%s, AMBIX_READ, info)", AMBIXTEST_FILE1);
  
  fail_if (ambix_close(ambix)!=AMBIX_ERR_SUCCESS, __LINE__, "File was not closed properly");
    
  free(info);
  return 0;
}
