/* simple - test ambix simple

   Copyright © 2012 IOhannes m zmölnig <zmoelnig@iem.at>.
         Institute of Electronic Music and Acoustics (IEM),
         University of Music and Dramatic Arts, Graz

   This file is part of libambix

   libambix is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   Libgcrypt is distributed in the hope that it will be useful,
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
  float32_t periods=20000;
  uint32_t err32;
  float32_t diff=0.;

  printf("test using '%s' [%d]\n", path, (int)format);

  resultdata=calloc(channels*frames, sizeof(float32_t));
  data=calloc(channels*frames, sizeof(float32_t));

  memset(&winfo, 0, sizeof(winfo));
  memset(&info, 0, sizeof(info));

  info.fileformat=AMBIX_SIMPLE;
  info.ambichannels=channels;
  info.extrachannels=0;
  info.samplerate=44100;
  info.sampleformat=format;

  memcpy(&rinfo, &info, sizeof(info));

  ambix=ambix_open(path, AMBIX_WRITE, &rinfo);
  fail_if((NULL==ambix), __LINE__, "couldn't create ambix file '%s' for writing", path);

  orgdata=data_sine(frames, channels, periods);
  //data_print(orgdata, 100);

  memcpy(data, orgdata, frames*channels*sizeof(float32_t));

  fail_if((NULL==data), __LINE__, "couldn't create data %dx%d sine @ %f", frames, channels, periods);

  err32=ambix_writef_float32(ambix, data, NULL, frames);
  fail_if((err32!=frames), __LINE__, "wrote only %d frames of %d", err32, frames);

  diff=data_diff(__LINE__, orgdata, data, frames*channels, eps);
  fail_if((diff>eps), __LINE__, "data diff %f > %f", diff, eps);

  fail_if((AMBIX_ERR_SUCCESS!=ambix_close(ambix)), __LINE__, "closing ambix file %p", ambix);
  ambix=NULL;

  ambix=ambix_open(path, AMBIX_READ, &rinfo);
  fail_if((NULL==ambix), __LINE__, "couldn't create ambix file '%s' for reading", path);

  fail_if((info.fileformat!=rinfo.fileformat), __LINE__, "fileformat mismatch %d!=%d", info.fileformat, rinfo.fileformat);
  fail_if((info.samplerate!=rinfo.samplerate), __LINE__, "samplerate mismatch %g!=%g", info.samplerate, rinfo.samplerate);
  fail_if((info.sampleformat!=rinfo.sampleformat), __LINE__, "sampleformat mismatch %d!=%d", info.sampleformat, rinfo.sampleformat);
  fail_if((info.ambichannels!=rinfo.ambichannels), __LINE__, "ambichannels mismatch %d!=%d", info.ambichannels, rinfo.ambichannels);
  fail_if((info.extrachannels!=rinfo.extrachannels), __LINE__, "extrachannels mismatch %d!=%d", info.extrachannels, rinfo.extrachannels);

  err32=ambix_readf_float32(ambix, resultdata, NULL, frames);
  fail_if((err32!=frames), __LINE__, "wrote only %d frames of %d", err32, frames);

  diff=data_diff(__LINE__, orgdata, resultdata, frames*channels, eps);
  fail_if((diff>eps), __LINE__, "data diff %f > %f", diff, eps);

  fail_if((AMBIX_ERR_SUCCESS!=ambix_close(ambix)), __LINE__, "closing ambix file %p", ambix);
  ambix=NULL;

  //  unlink(path);
}



int main(int argc, char**argv) {
  check_create_simple("test-float32.caf",  AMBIX_SAMPLEFORMAT_FLOAT32, 1e-7);
  check_create_simple("test-pcm32.caf",  AMBIX_SAMPLEFORMAT_PCM32, 1e-5);
  check_create_simple("test-pcm16.caf",  AMBIX_SAMPLEFORMAT_PCM16, 1./20000.);


  pass();
  return 0;
}
