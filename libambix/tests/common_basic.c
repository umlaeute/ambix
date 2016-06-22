/* simple - test ambix simple

   Copyright © 2012-2016 IOhannes m zmölnig <zmoelnig@iem.at>.
         Institute of Electronic Music and Acoustics (IEM),
         University of Music and Dramatic Arts, Graz

   This file is part of libambix

   libambix is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   libambix is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"
#include <unistd.h>
#include <string.h>


void check_create_simple(const char*path, ambix_sampleformat_t format, float32_t eps) {
  ambix_info_t info, rinfo, winfo;
  ambix_t*ambix=NULL;
  float32_t*orgdata,*data,*resultdata;
  uint32_t frames=441000;
  uint32_t channels=4;
  float32_t periods=4724;
  int64_t err64;
  float32_t diff=0.;
  uint32_t gotframes;

  printf("test using '%s' [%d]\n", path, (int)format);

  resultdata=(float32_t*)calloc(channels*frames, sizeof(float32_t));
  data=(float32_t*)calloc(channels*frames, sizeof(float32_t));

  memset(&winfo, 0, sizeof(winfo));
  memset(&info, 0, sizeof(info));

  info.fileformat=AMBIX_BASIC;
  info.ambichannels=channels;
  info.extrachannels=0;
  info.samplerate=44100;
  info.sampleformat=format;

  memcpy(&rinfo, &info, sizeof(info));

  ambix=ambix_open(path, AMBIX_WRITE, &rinfo);
  fail_if((NULL==ambix), __LINE__, "couldn't create ambix file '%s' for writing", path);

  orgdata=data_sine(FLOAT32, frames, channels, periods);
  //data_print(FLOAT32, orgdata, 100);

  memcpy(data, orgdata, frames*channels*sizeof(float32_t));

  fail_if((NULL==data), __LINE__, "couldn't create data %dx%d sine @ %f", (int)frames, (int)channels, (float)periods);

  err64=ambix_writef_float32(ambix, data, NULL, frames);
  fail_if((err64!=frames), __LINE__, "wrote only %d frames of %d", (int)err64, (int)frames);

  diff=data_diff(__LINE__, FLOAT32, orgdata, data, frames*channels, eps);
  fail_if((diff>eps), __LINE__, "data diff %f > %f", diff, eps);

  fail_if((AMBIX_ERR_SUCCESS!=ambix_close(ambix)), __LINE__, "closing ambix file %p", ambix);
  ambix=NULL;

  ambix=ambix_open(path, AMBIX_READ, &rinfo);
  fail_if((NULL==ambix), __LINE__, "couldn't create ambix file '%s' for reading", path);

  fail_if((info.fileformat!=rinfo.fileformat), __LINE__, "fileformat mismatch %d!=%d", (int)info.fileformat, (int)rinfo.fileformat);
  fail_if((info.samplerate!=rinfo.samplerate), __LINE__, "samplerate mismatch %g!=%g", (float)info.samplerate, (float)rinfo.samplerate);
  fail_if((info.sampleformat!=rinfo.sampleformat), __LINE__, "sampleformat mismatch %d!=%d", (int)info.sampleformat, (int)rinfo.sampleformat);
  fail_if((info.ambichannels!=rinfo.ambichannels), __LINE__, "ambichannels mismatch %d!=%d", (int)info.ambichannels, (int)rinfo.ambichannels);
  fail_if((info.extrachannels!=rinfo.extrachannels), __LINE__, "extrachannels mismatch %d!=%d", (int)info.extrachannels, (int)rinfo.extrachannels);

  gotframes=0;
  do {
    err64=ambix_readf_float32(ambix, resultdata+(gotframes*channels), NULL, (frames-gotframes));
    fail_if((err64<0), __LINE__, "reading frames failed after %d/%d frames", (int)gotframes, (int)frames);
    gotframes+=err64;
  } while(err64>0 && gotframes<frames);

  diff=data_diff(__LINE__, FLOAT32, orgdata, resultdata, frames*channels, eps);
  fail_if((diff>eps), __LINE__, "data diff %f > %f", diff, eps);

  fail_if((AMBIX_ERR_SUCCESS!=ambix_close(ambix)), __LINE__, "closing ambix file %p", ambix);
  ambix=NULL;

  free(data);
  free(resultdata);
  free(orgdata);

  ambixtest_rmfile(path);
}
