/* extended - test ambix extended

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


void check_create_extended(const char*path, ambix_sampleformat_t format, uint32_t chunksize, float32_t eps) {
  ambix_info_t info, rinfo, winfo;
  ambix_t*ambix=NULL;
  float32_t*orgambidata,*ambidata,*resultambidata;
  float32_t*orgotherdata,*otherdata,*resultotherdata;
  uint32_t framesize=441000;
  uint32_t ambichannels=4;
  uint32_t extrachannels=2;
  float32_t periods=20000;
  ambix_matrix_t eye={0,0,NULL};
  const ambix_matrix_t*eye2=NULL;
  int64_t err64, gotframes;
  float32_t diff=0.;
  STARTTEST("\n");

  printf("test using '%s' [%d] with chunks of %d and eps=%f\n", path, (int)format, (int)chunksize, eps);

  resultambidata=(float32_t*)calloc(ambichannels*framesize, sizeof(float32_t));
  ambidata=(float32_t*)calloc(ambichannels*framesize, sizeof(float32_t));

  resultotherdata=(float32_t*)calloc(extrachannels*framesize, sizeof(float32_t));
  otherdata=(float32_t*)calloc(extrachannels*framesize, sizeof(float32_t));

  ambix_matrix_init(ambichannels, ambichannels, &eye);
  ambix_matrix_fill(&eye, AMBIX_MATRIX_IDENTITY);

  memset(&winfo, 0, sizeof(winfo));
  memset(&info, 0, sizeof(info));

  info.fileformat=AMBIX_EXTENDED;
  info.ambichannels=ambichannels;
  info.extrachannels=extrachannels;
  info.samplerate=44100;
  info.sampleformat=format;

  memcpy(&rinfo, &info, sizeof(info));

  ambix=ambix_open(path, AMBIX_WRITE, &rinfo);
  fail_if((NULL==ambix), __LINE__, "couldn't create ambix file '%s' for writing", path);

  orgambidata=data_sine (FLOAT32, framesize, ambichannels, periods);
  orgotherdata=data_ramp(FLOAT32, framesize, extrachannels);
  //data_print(FLOAT32, orgdata, 100);
  fail_if((NULL==orgambidata), __LINE__, "couldn't create ambidata %dx%d sine @ %f", (int)framesize, (int)ambichannels, (float)periods);
  fail_if((NULL==orgotherdata), __LINE__, "couldn't create otherdata %dx%d sine @ %f", (int)framesize, (int)extrachannels, (float)periods);

  memcpy(ambidata, orgambidata, framesize*ambichannels*sizeof(float32_t));
  memcpy(otherdata, orgotherdata, framesize*extrachannels*sizeof(float32_t));

  fail_if((AMBIX_ERR_SUCCESS!=ambix_set_adaptormatrix(ambix, &eye)),
          __LINE__, "failed setting adaptor matrix");

  if(chunksize>0) {
    uint32_t subframe=chunksize;
    uint32_t chunks = framesize/chunksize;
    uint32_t framesleft=framesize;
    uint32_t frame;
    printf("writing %d chunks of %d frames\n", (int)chunks, (int)chunksize);
    for(frame=0; frame<chunks; frame++) {
      err64=ambix_writef_float32(ambix, ambidata+ambichannels*frame*chunksize, otherdata+extrachannels*frame*chunksize, chunksize);
      fail_if((err64!=chunksize), __LINE__, "wrote only %d chunksize of %d", (int)err64, (int)chunksize);
      framesleft-=chunksize;
    }
    subframe=framesleft;
    printf("writing rest of %d frames\n", (int)subframe);
    err64=ambix_writef_float32(ambix, ambidata+ambichannels*frame*chunksize, otherdata+extrachannels*frame*chunksize, subframe);
    fail_if((err64!=subframe), __LINE__, "wrote only %d subframe of %d", (int)err64, (int)subframe);

  } else {
    err64=ambix_writef_float32(ambix, ambidata, otherdata, framesize);
    fail_if((err64!=framesize), __LINE__, "wrote only %d frames of %d", (int)err64, (int)framesize);
  }

  diff=data_diff(__LINE__, FLOAT32, orgambidata, ambidata, framesize*ambichannels, eps);
  fail_if((diff>eps), __LINE__, "ambidata diff %f > %f", diff, eps);
  diff=data_diff(__LINE__, FLOAT32, orgotherdata, otherdata, framesize*extrachannels, eps);
  fail_if((diff>eps), __LINE__, "otherdata diff %f > %f", diff, eps);

  fail_if((AMBIX_ERR_SUCCESS!=ambix_close(ambix)), __LINE__, "closing ambix file %p", ambix);
  ambix=NULL;



  /* read data back */
  ambix=ambix_open(path, AMBIX_READ, &rinfo);
  fail_if((NULL==ambix), __LINE__, "couldn't create ambix file '%s' for reading", path);

  fail_if((info.fileformat!=rinfo.fileformat), __LINE__, "fileformat mismatch %d!=%d", (int)info.fileformat, (int)rinfo.fileformat);
  fail_if((info.samplerate!=rinfo.samplerate), __LINE__, "samplerate mismatch %g!=%g", (float)info.samplerate, (float)rinfo.samplerate);
  fail_if((info.sampleformat!=rinfo.sampleformat), __LINE__, "sampleformat mismatch %d!=%d", (int)info.sampleformat, (int)rinfo.sampleformat);
  fail_if((info.ambichannels!=rinfo.ambichannels), __LINE__, "ambichannels mismatch %d!=%d", (int)info.ambichannels, (int)rinfo.ambichannels);
  fail_if((info.extrachannels!=rinfo.extrachannels), __LINE__, "extrachannels mismatch %d!=%d", (int)info.extrachannels, (int)rinfo.extrachannels);

  eye2=ambix_get_adaptormatrix(ambix);
  fail_if((NULL==eye2), __LINE__, "failed reading adaptor matrix");

  diff=matrix_diff(__LINE__, &eye, eye2, eps);
  fail_if((diff>eps), __LINE__, "adaptormatrix diff %f > %f", diff, eps);


  gotframes=0;
  do {
    //err64=ambix_readf_float32(ambix, resultambidata, resultotherdata, framesize);
      err64=ambix_readf_float32(ambix,
			resultambidata +(gotframes*ambichannels ),
			resultotherdata+(gotframes*extrachannels),
			(framesize-gotframes));
    fail_if((err64<0), __LINE__, "reading frames failed after %d/%d frames", (int)gotframes, (int)framesize);
    gotframes+=err64;
  } while(err64>0 && gotframes<framesize);

  diff=data_diff(__LINE__, FLOAT32, orgambidata, resultambidata, framesize*ambichannels, eps);
  fail_if((diff>eps), __LINE__, "ambidata diff %f > %f", diff, eps);

  diff=data_diff(__LINE__, FLOAT32, orgotherdata, resultotherdata, framesize*extrachannels, eps);
  fail_if((diff>eps), __LINE__, "otherdata diff %f > %f", diff, eps);

  fail_if((AMBIX_ERR_SUCCESS!=ambix_close(ambix)), __LINE__, "closing ambix file %p", ambix);
  ambix=NULL;

  free(resultambidata);
  free(ambidata);
  free(resultotherdata);
  free(otherdata);

  free(orgambidata);
  free(orgotherdata);

  ambix_matrix_deinit(&eye);

  ambixtest_rmfile(path);
}
