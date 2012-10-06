#include "common.h"
#include "data.h"

int main()
{
  ambix_t*ambix = NULL;
  
  ambix_info_t *info= calloc(1, sizeof(ambix_info_t));
  
  ambix=ambix_open(AMBIXTEST_FILE1, AMBIX_READ, info);
  
  fail_if(NULL==ambix, __LINE__, "File was not open");  
  
  fail_if (info->frames != 44100, __LINE__, "Frames are not correct");
  fail_if (info->samplerate != 44100, __LINE__, "Samplerate is not correct");
  fail_if (info->sampleformat != AMBIX_SAMPLEFORMAT_FLOAT32, __LINE__, "Sampleformat is not correct");
  fail_if (info->fileformat != AMBIX_BASIC, __LINE__, "Fileformat is not correct");  
  fail_if (info->ambichannels != 9, __LINE__, "Ambichannels are not correct");    
  fail_if (info->extrachannels != 0, __LINE__, "Non-ambichannels are not correct");
  fail_if (NULL!=ambix_get_adaptormatrix(ambix), __LINE__, "Error, there is reconstruction matrix");
  
  ambix_close (ambix);
  free(info);
  return 0;
}
