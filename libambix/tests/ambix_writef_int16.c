#include "common.h"
#include "data.h"

int main ()
{
  ambix_t *ambix = NULL;
  ambix_info_t *info= calloc(1, sizeof(ambix_info_t));
  int16_t *ambidata= 0;
  int16_t *otherdata=0;
  int16_t frames= 10;
  
  
  ambix=ambix_open(AMBIXTEST_FILE1, AMBIX_READ, info);
  int16_t result=0;
  
  /* ambix_writef_int16 (ambix_t *ambix, const int16_t *ambidata, const int16_t *otherdata, int64_t frames)
     Write (16bit signed integer) samples to the ambix file. 
  Returns:
    the number of sample frames successfully written 
    */
  
  // UNCOMMENT WHEN FIXED
  /*result=ambix_writef_int16 (ambix,ambidata,otherdata,frames);
  fail_if (frames!=result, __LINE__, "Files were not written successfully");
  */
  
  ambix_close(ambix);
  free(info);

  /* FIXXME: no test yet */
  return skip();
}
