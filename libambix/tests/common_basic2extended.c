/* common_simple2exetended - common function for writing extended files as simple files

   Copyright © 2016 IOhannes m zmölnig <zmoelnig@iem.at>.
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

int check_create_b2e(const char*path, ambix_sampleformat_t format,
		      ambix_matrix_t*matrix, uint32_t extrachannels,
		      uint32_t chunksize, float32_t eps) {
  uint32_t fullambichannels = matrix?matrix->rows:0;
  uint32_t ambixchannels = matrix?matrix->cols:0;
  ambix_info_t info, rinfo, winfo;
  ambix_t*ambix=NULL;
  float32_t*orgambidata,*ambidata,*resultambidata;
  float32_t*orgotherdata,*otherdata,*resultotherdata;
  uint32_t framesize=441000;
  float32_t periods=20000;
  const ambix_matrix_t*mtx2=NULL;
  int64_t err64, gotframes;
  float32_t diff=0.;
  ambix_err_t err=0;
  STARTTEST("ambi=[%dx%d],extra=%d, format=%d\n",
	    matrix?matrix->rows:-1, matrix?matrix->cols:-1, extrachannels, format);

  //printf("test using '%s' [%d] with chunks of %d and eps=%f\n", path, (int)format, (int)chunksize, eps);

  resultambidata=(float32_t*)calloc(fullambichannels*framesize, sizeof(float32_t));
  ambidata=(float32_t*)calloc(fullambichannels*framesize, sizeof(float32_t));

  resultotherdata=(float32_t*)calloc(extrachannels*framesize, sizeof(float32_t));
  otherdata=(float32_t*)calloc(extrachannels*framesize, sizeof(float32_t));

  memset(&info, 0, sizeof(info));
  info.fileformat=AMBIX_EXTENDED;
  info.ambichannels=fullambichannels; // FIXXME ??
  info.extrachannels=extrachannels;
  info.samplerate=44100;
  info.sampleformat=format;

  memcpy(&winfo, &info, sizeof(info));
  /* we want to write an EXTENDED file using the BASIC api (full set) */
  winfo.fileformat=AMBIX_BASIC;

  ambix=ambix_open(path, AMBIX_WRITE, &winfo);
  if(fail_if((NULL==ambix), __LINE__, "couldn't create ambix file '%s' for writing", path))return 1;

  orgambidata=data_sine(framesize, fullambichannels, periods);
  orgotherdata=data_ramp(framesize, extrachannels);
  //data_print(orgdata, 100);
  if(fail_if((NULL==orgambidata), __LINE__, "couldn't create ambidata %dx%d sine @ %f", (int)framesize, (int)fullambichannels, (float)periods))return 1;
  if(fail_if((NULL==orgotherdata), __LINE__, "couldn't create otherdata %dx%d sine @ %f", (int)framesize, (int)extrachannels, (float)periods))return 1;

  memcpy(ambidata, orgambidata, framesize*fullambichannels*sizeof(float32_t));
  memcpy(otherdata, orgotherdata, framesize*extrachannels*sizeof(float32_t));

  err=ambix_set_adaptormatrix(ambix, matrix);
  if(1 || AMBIX_ERR_SUCCESS!=err) {
    ambix_matrix_t*pinv=0;
    //printf("adaptor matrix:\n"); matrix_print(matrix);
    pinv=ambix_matrix_pinv(matrix, pinv);
    if(pinv) {
      //printf("pseudo-inverse:\n"); matrix_print(pinv);
    } else {
      //printf("no pseudo-inverse!\n");
      ;
    }
  }
  if(fail_if((AMBIX_ERR_SUCCESS!=err),
	     __LINE__, "failed setting adaptor matrix [%d]", err))return 1;

  if(chunksize>0) {
    uint32_t subframe=chunksize;
    uint32_t chunks = framesize/chunksize;
    uint32_t framesleft=framesize;
    uint32_t frame;
    //printf("writing %d chunks of %d frames\n", (int)chunks, (int)chunksize);
    for(frame=0; frame<chunks; frame++) {
      err64=ambix_writef_float32(ambix, ambidata+fullambichannels*frame*chunksize, otherdata+extrachannels*frame*chunksize, chunksize);
      if(fail_if((err64!=chunksize), __LINE__, "wrote only %d chunksize of %d", (int)err64, (int)chunksize))return 1;
      framesleft-=chunksize;
    }
    subframe=framesleft;
    //printf("writing rest of %d frames\n", (int)subframe);
    err64=ambix_writef_float32(ambix, ambidata+fullambichannels*frame*chunksize, otherdata+extrachannels*frame*chunksize, subframe);
    if(fail_if((err64!=subframe), __LINE__, "wrote only %d subframe of %d", (int)err64, (int)subframe))return 1;

  } else {
    err64=ambix_writef_float32(ambix, ambidata, otherdata, framesize);
    if(fail_if((err64!=framesize), __LINE__, "wrote only %d frames of %d", (int)err64, (int)framesize))return 1;
  }

  diff=data_diff(__LINE__, orgambidata, ambidata, framesize*fullambichannels, eps);
  if(fail_if((diff>eps), __LINE__, "ambidata diff %f > %f", diff, eps))return 1;
  diff=data_diff(__LINE__, orgotherdata, otherdata, framesize*extrachannels, eps);
  if(fail_if((diff>eps), __LINE__, "otherdata diff %f > %f", diff, eps))return 1;

  if(fail_if((AMBIX_ERR_SUCCESS!=ambix_close(ambix)), __LINE__, "closing ambix file %p", ambix))return 1;
  ambix=NULL;



  /* read data back via BASIC */
  STARTTEST("readback BASIC\n");
  memset(&rinfo, 0, sizeof(rinfo));
  rinfo.fileformat = AMBIX_BASIC;
  ambix=ambix_open(path, AMBIX_READ, &rinfo);
  if(fail_if((NULL==ambix), __LINE__, "couldn't open ambix file '%s' for reading", path))return 1;

  if(fail_if((AMBIX_BASIC!=rinfo.fileformat), __LINE__, "fileformat mismatch %d!=%d", (int)AMBIX_BASIC, (int)rinfo.fileformat))return 1;
  if(fail_if((info.samplerate!=rinfo.samplerate), __LINE__, "samplerate mismatch %g!=%g", (float)info.samplerate, (float)rinfo.samplerate))return 1;
  if(fail_if((info.sampleformat!=rinfo.sampleformat), __LINE__, "sampleformat mismatch %d!=%d", (int)info.sampleformat, (int)rinfo.sampleformat))return 1;
  if(fail_if((info.ambichannels!=rinfo.ambichannels), __LINE__, "ambichannels mismatch %d!=%d", (int)info.ambichannels, (int)rinfo.ambichannels))return 1;
  if(fail_if((info.extrachannels!=rinfo.extrachannels), __LINE__, "extrachannels mismatch %d!=%d", (int)info.extrachannels, (int)rinfo.extrachannels))return 1;

  gotframes=0;
  do {
    //err64=ambix_readf_float32(ambix, resultambidata, resultotherdata, framesize);
      err64=ambix_readf_float32(ambix,
			resultambidata +(gotframes*fullambichannels ),
			resultotherdata+(gotframes*extrachannels),
			(framesize-gotframes));
    if(fail_if((err64<0), __LINE__, "reading frames failed after %d/%d frames", (int)gotframes, (int)framesize))return 1;
    gotframes+=err64;
  } while(err64>0 && gotframes<framesize);

  diff=data_diff(__LINE__, orgambidata, resultambidata, framesize*fullambichannels, eps);
  if(fail_if((diff>eps), __LINE__, "ambidata diff %f > %f", diff, eps))return 1;

  diff=data_diff(__LINE__, orgotherdata, resultotherdata, framesize*extrachannels, eps);
  if(fail_if((diff>eps), __LINE__, "otherdata diff %f > %f", diff, eps))return 1;

  if(fail_if((AMBIX_ERR_SUCCESS!=ambix_close(ambix)), __LINE__, "closing ambix file %p", ambix))return 1;
  ambix=NULL;


  /* read data back via EXTENDED */
  STARTTEST("readback EXTENDED\n");
  memset(&rinfo, 0, sizeof(rinfo));
  rinfo.fileformat = AMBIX_EXTENDED;

  ambix=ambix_open(path, AMBIX_READ, &rinfo);
  if(fail_if((NULL==ambix), __LINE__, "couldn't open ambix file '%s' for reading", path))return 1;

  if(fail_if((AMBIX_EXTENDED!=rinfo.fileformat), __LINE__, "fileformat mismatch %d!=%d", (int)info.fileformat, (int)rinfo.fileformat))return 1;
  if(fail_if((info.samplerate!=rinfo.samplerate), __LINE__, "samplerate mismatch %g!=%g", (float)info.samplerate, (float)rinfo.samplerate))return 1;
  if(fail_if((info.sampleformat!=rinfo.sampleformat), __LINE__, "sampleformat mismatch %d!=%d", (int)info.sampleformat, (int)rinfo.sampleformat))return 1;
  if(fail_if((info.ambichannels!=rinfo.ambichannels), __LINE__, "ambichannels mismatch %d!=%d", (int)info.ambichannels, (int)rinfo.ambichannels))return 1;
  if(fail_if((info.extrachannels!=rinfo.extrachannels), __LINE__, "extrachannels mismatch %d!=%d", (int)info.extrachannels, (int)rinfo.extrachannels))return 1;

  mtx2=ambix_get_adaptormatrix(ambix);
  if(fail_if((NULL==mtx2), __LINE__, "failed reading adaptor matrix"))return 1;

  diff=matrix_diff(__LINE__, matrix, mtx2, eps);
  if(fail_if((diff>eps), __LINE__, "adaptormatrix diff %f > %f", diff, eps))return 1;

  gotframes=0;
  do {
    //err64=ambix_readf_float32(ambix, resultambidata, resultotherdata, framesize);
      err64=ambix_readf_float32(ambix,
			resultambidata +(gotframes*ambixchannels ),
			resultotherdata+(gotframes*extrachannels),
			(framesize-gotframes));
    if(fail_if((err64<0), __LINE__, "reading frames failed after %d/%d frames", (int)gotframes, (int)framesize))return 1;
    gotframes+=err64;
  } while(err64>0 && gotframes<framesize);
#if 0
  diff=data_diff(__LINE__, orgambidata, resultambidata, framesize*ambixchannels, eps);
  if(fail_if((diff>eps), __LINE__, "ambidata diff %f > %f", diff, eps))return 1;
#endif
  diff=data_diff(__LINE__, orgotherdata, resultotherdata, framesize*extrachannels, eps);
  if(fail_if((diff>eps), __LINE__, "otherdata diff %f > %f", diff, eps))return 1;

  if(fail_if((AMBIX_ERR_SUCCESS!=ambix_close(ambix)), __LINE__, "closing ambix file %p", ambix))return 1;
  ambix=NULL;



  free(resultambidata);
  free(ambidata);
  free(resultotherdata);
  free(otherdata);

  free(orgambidata);
  free(orgotherdata);

  unlink(path);
  return 0;
}
