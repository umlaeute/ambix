#include "common.h"
#include "data.h"

int main()
{
  
  ambix_t *ambix = NULL;
  ambix_info_t *info= calloc(1, sizeof(ambix_info_t));
  int32_t ambidata[100];
  int32_t *otherdata= 0;
 
  int result=0;
  int cmp=0;
  
   ambix=ambix_open(AMBIXTEST_FILE1, AMBIX_READ, info);
  //ambix_readf_int32 (ambix_t *ambix, int32_t *ambidata, int32_t *otherdata, int64_t frames)
  //Read samples (as 32bit signed integer values) from the ambix file. 
  
  
  //UNCOMMENT WHEN FIXED
  /*cmp= ambix_readf_int32(ambix, *ambidata, otherdata, 100);
  printf("rezultat readf-a je %d\n", cmp);
  result= ambix_seek (ambix, 100, SEEK_CUR);
  
  if (cmp!=result)
  {
  printf ("Greska u potrazi\n"); 
  }
  else
  fail_if (result!=100, __LINE__, "File failed to seek correct data");*/
  
  ambix_close(ambix);
  free(info);

  /* FIXXXME: this test is not working yet */
  return skip();
}
