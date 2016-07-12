#include "common.h"
#include "data.h"

int main()
{
  ambix_t*ambix = NULL;
  struct SNDFILE_tag *sndfile = NULL;
  ambix_info_t *info= calloc(1, sizeof(ambix_info_t));

  ambix=ambix_open(AMBIXTEST_FILE1, AMBIX_READ, info);

  fail_if(NULL==ambix, __LINE__, "File was not open");
  sndfile = ambix_get_sndfile (ambix);

#ifdef HAVE_SNDFILE
#warning LATER: skip, once we have multiple (working) backends
  /* there is no need to use libsndfile even if libsndfile is present */
  fail_if(NULL==sndfile, __LINE__, "no sndfile handle despite using libsndfile");
#else
  fail_if(NULL!=sndfile, __LINE__, "got sndfile handle without libsndfile!");
#endif

  ambix_close (ambix);
  free(info);
  return 0;
}
