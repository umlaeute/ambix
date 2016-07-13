#include "common.h"
#include "data.h"

#include <string.h>

int main(int argc, char**argv) {
  const char*markerfile=FILENAME_FILE;
  ambix_t*ambix=NULL;
  ambix_info_t info;
  memset(&info, 0, sizeof(info));
  uint32_t frames=441000;
  uint32_t channels=4;
  int64_t err64;
  float32_t*data;
  
  info.fileformat=AMBIX_BASIC;
  info.ambichannels=channels;
  info.extrachannels=0;
  info.samplerate=44100;
  info.sampleformat=AMBIX_SAMPLEFORMAT_FLOAT32;
  
  data=(float32_t*)calloc(channels*frames, sizeof(float32_t));
  
  ambix=ambix_open(markerfile, AMBIX_WRITE, &info);
  fail_if(NULL==ambix, __LINE__, "File was not open");

  /* add some markers */
  ambix_marker_t marker_1;
  memset(&marker_1, 0, sizeof(ambix_marker_t));
  marker_1.position = 1.0;
  strncpy(marker_1.name, "this is marker #1", 255);

  ambix_marker_t marker_2;
  memset(&marker_2, 0, sizeof(ambix_marker_t));
  marker_2.position = 2.0;
  strncpy(marker_2.name, "this is marker #2", 255);


  ambix_marker_t marker_3;
  memset(&marker_3, 0, sizeof(ambix_marker_t));
  marker_3.position = 3.0;

  fail_if(0!=ambix_add_marker(ambix, &marker_1), __LINE__, "Could not add marker");
  fail_if(0!=ambix_add_marker(ambix, &marker_2), __LINE__, "Could not add marker");
  fail_if(0!=ambix_add_marker(ambix, &marker_3), __LINE__, "Could not add marker");

  /* add some regions */
  ambix_region_t region_1;
  memset(&region_1, 0, sizeof(ambix_region_t));
  region_1.start_position = 1.0;
  region_1.end_position = 2.0;
  strncpy(region_1.name, "this is region #1", 255);

  ambix_region_t region_2;
  memset(&region_2, 0, sizeof(ambix_region_t));
  region_2.start_position = 3.0;
  region_2.end_position = 4.0;
  strncpy(region_2.name, "this is region #2", 255);

  ambix_region_t region_3;
  memset(&region_3, 0, sizeof(ambix_region_t));
  region_3.start_position = 3.0;
  region_3.end_position = 4.0;

  fail_if(0!=ambix_add_region(ambix, &region_1), __LINE__, "Could not add region");
  fail_if(0!=ambix_add_region(ambix, &region_2), __LINE__, "Could not add region");
  fail_if(0!=ambix_add_region(ambix, &region_3), __LINE__, "Could not add region");

  /* write testsamples */
  err64=ambix_writef_float32(ambix, data, NULL, frames);

  ambix_close(ambix);


  /* open the file again and see wheter the markers and regions have been saved and can be read */
  memset(&info, 0, sizeof(info));
  ambix=ambix_open(markerfile, AMBIX_READ, &info);

  fail_if(0 == ambix_get_num_markers(ambix), __LINE__, "No markers in file");
  fail_if(0 == ambix_get_num_regions(ambix), __LINE__, "No regions in file");

  ambix_marker_t *marker_1_retr;
  ambix_marker_t *marker_2_retr;
  ambix_marker_t *marker_3_retr;

  ambix_region_t *region_1_retr;
  ambix_region_t *region_2_retr;
  ambix_region_t *region_3_retr;

  marker_1_retr=ambix_get_marker(ambix, 0);
  marker_2_retr=ambix_get_marker(ambix, 1);
  marker_3_retr=ambix_get_marker(ambix, 2);

  region_1_retr=ambix_get_region(ambix, 0);
  region_2_retr=ambix_get_region(ambix, 1);
  region_3_retr=ambix_get_region(ambix, 2);

  fail_if(marker_1_retr==NULL, __LINE__, "Could not retrieve marker 1");
  fail_if(marker_2_retr==NULL, __LINE__, "Could not retrieve marker 2");
  fail_if(marker_3_retr==NULL, __LINE__, "Could not retrieve marker 3");
  fail_if(region_1_retr==NULL, __LINE__, "Could not retrieve region 1");
  fail_if(region_2_retr==NULL, __LINE__, "Could not retrieve region 2");
  fail_if(region_3_retr==NULL, __LINE__, "Could not retrieve region 3");

  fail_if(memcmp(&marker_1, marker_1_retr, sizeof(ambix_marker_t)), __LINE__, "Marker 1 does not match");
  fail_if(memcmp(&marker_2, marker_2_retr, sizeof(ambix_marker_t)), __LINE__, "Marker 2 does not match");
  fail_if(memcmp(&marker_3, marker_3_retr, sizeof(ambix_marker_t)), __LINE__, "Marker 3 does not match");
  fail_if(memcmp(&region_1, region_1_retr, sizeof(ambix_region_t)), __LINE__, "Region 1 does not match");
  fail_if(memcmp(&region_2, region_2_retr, sizeof(ambix_region_t)), __LINE__, "Region 2 does not match");
  fail_if(memcmp(&region_3, region_3_retr, sizeof(ambix_region_t)), __LINE__, "Region 3 does not match");

  if(data)
    free(data);
  ambix_close(ambix);
  ambixtest_rmfile(markerfile);
  return pass();
}
