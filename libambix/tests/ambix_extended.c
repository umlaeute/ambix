/* extended - test ambix extended

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


void check_create_extended(const char*path, ambix_sampleformat_t format, float32_t eps) {
  ambixinfo_t info, rinfo, winfo;
  ambix_t*ambix=NULL;
  float32_t*orgambidata,*ambidata,*resultambidata;
  float32_t*orgotherdata,*otherdata,*resultotherdata;
  uint32_t frames=441000;
  uint32_t ambichannels=4;
  uint32_t otherchannels=2;
  float32_t periods=20000;
  ambixmatrix_t eye={0,0,NULL};
  const ambixmatrix_t*eye2=NULL;
  uint32_t err32;
  float32_t diff=0.;

  printf("test using '%s' [%d]\n", path, (int)format);

  resultambidata=calloc(ambichannels*frames, sizeof(float32_t));
  ambidata=calloc(ambichannels*frames, sizeof(float32_t));

  resultotherdata=calloc(otherchannels*frames, sizeof(float32_t));
  otherdata=calloc(otherchannels*frames, sizeof(float32_t));

  ambix_matrix_init(ambichannels, ambichannels, &eye);
  ambix_matrix_eye(&eye);

  memset(&winfo, 0, sizeof(winfo));
  memset(&info, 0, sizeof(info));

  info.ambixfileformat=AMBIX_EXTENDED;
  info.ambichannels=ambichannels;
  info.otherchannels=otherchannels;
  info.samplerate=44100;
  info.sampleformat=format;

  memcpy(&rinfo, &info, sizeof(info));

  ambix=ambix_open(path, AMBIX_WRITE, &rinfo);
  fail_if((NULL==ambix), __LINE__, "couldn't create ambix file '%s' for writing", path);

  orgambidata=data_sine(frames, ambichannels, periods);
  orgotherdata=data_ramp(frames, otherchannels);
  //data_print(orgdata, 100);
  fail_if((NULL==orgambidata), __LINE__, "couldn't create ambidata %dx%d sine @ %f", frames, ambichannels, periods);
  fail_if((NULL==orgotherdata), __LINE__, "couldn't create otherdata %dx%d sine @ %f", frames, otherchannels, periods);

  memcpy(ambidata, orgambidata, frames*ambichannels*sizeof(float32_t));
  memcpy(otherdata, orgotherdata, frames*otherchannels*sizeof(float32_t));

  fail_if((AMBIX_ERR_SUCCESS!=ambix_setAdaptorMatrix(ambix, &eye)),
          __LINE__, "failed setting adaptor matrix");
  fail_if((AMBIX_ERR_SUCCESS!=ambix_write_header(ambix)),
          __LINE__, "failed writing headers");

  err32=ambix_writef_float32(ambix, ambidata, otherdata, frames);
  fail_if((err32!=frames), __LINE__, "wrote only %d frames of %d", err32, frames);

  diff=data_diff(__LINE__, orgambidata, ambidata, frames*ambichannels, eps);
  fail_if((diff>eps), __LINE__, "ambidata diff %f > %f", diff, eps);
  diff=data_diff(__LINE__, orgotherdata, otherdata, frames*otherchannels, eps);
  fail_if((diff>eps), __LINE__, "otherdata diff %f > %f", diff, eps);

  fail_if((AMBIX_ERR_SUCCESS!=ambix_close(ambix)), __LINE__, "closing ambix file %p", ambix);
  ambix=NULL;



  /* read data back */
  ambix=ambix_open(path, AMBIX_READ, &rinfo);
  fail_if((NULL==ambix), __LINE__, "couldn't create ambix file '%s' for reading", path);

  fail_if((info.ambixfileformat!=rinfo.ambixfileformat), __LINE__, "ambixfileformat mismatch %d!=%d", info.ambixfileformat, rinfo.ambixfileformat);
  fail_if((info.samplerate!=rinfo.samplerate), __LINE__, "samplerate mismatch %g!=%g", info.samplerate, rinfo.samplerate);
  fail_if((info.sampleformat!=rinfo.sampleformat), __LINE__, "sampleformat mismatch %d!=%d", info.sampleformat, rinfo.sampleformat);
  fail_if((info.ambichannels!=rinfo.ambichannels), __LINE__, "ambichannels mismatch %d!=%d", info.ambichannels, rinfo.ambichannels);
  fail_if((info.otherchannels!=rinfo.otherchannels), __LINE__, "otherchannels mismatch %d!=%d", info.otherchannels, rinfo.otherchannels);

  eye2=ambix_getAdaptorMatrix(ambix);
  fail_if((NULL==eye2), __LINE__, "failed reading adaptor matrix");

  diff=matrix_diff(__LINE__, &eye, eye2, eps);
  fail_if((diff>eps), __LINE__, "adaptormatrix diff %f > %f", diff, eps);

  err32=ambix_readf_float32(ambix, resultambidata, resultotherdata, frames);
  fail_if((err32!=frames), __LINE__, "read only %d frames of %d", err32, frames);

  diff=data_diff(__LINE__, orgambidata, resultambidata, frames*ambichannels, eps);
  fail_if((diff>eps), __LINE__, "ambidata diff %f > %f", diff, eps);

  diff=data_diff(__LINE__, orgotherdata, resultotherdata, frames*otherchannels, eps);
  fail_if((diff>eps), __LINE__, "otherdata diff %f > %f", diff, eps);

  fail_if((AMBIX_ERR_SUCCESS!=ambix_close(ambix)), __LINE__, "closing ambix file %p", ambix);
  ambix=NULL;

  free(resultambidata);
  free(ambidata);
  free(resultotherdata);
  free(otherdata);
  //  unlink(path);
}



int main(int argc, char**argv) {
  check_create_extended("test2-float32.caf",  AMBIX_SAMPLEFORMAT_FLOAT32, 1e-7);
  check_create_extended("test2-pcm32.caf",  AMBIX_SAMPLEFORMAT_PCM32, 1e-5);
  check_create_extended("test2-pcm16.caf",  AMBIX_SAMPLEFORMAT_PCM16, 1./20000.);


  pass();
  return 0;
}
