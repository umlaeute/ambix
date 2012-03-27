/* ambix_test -  test ambix              -*- c -*-

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

#include "ambix/ambix.h"

#include <stdio.h>
#include <string.h>

void createfile_basic(const char*path, uint32_t ambichannels, uint32_t extrachannels, uint64_t frames) {
  ambix_info_t info;
  ambix_t*ambix;
  const ambix_matrix_t*matrix;

  extrachannels=0;

  memset(&info, 0, sizeof(info));
  info.frames=frames;
  info.samplerate=44100;
  info.sampleformat=AMBIX_SAMPLEFORMAT_FLOAT32;
  info.fileformat=AMBIX_BASIC;
  info.ambichannels=ambichannels;
  info.extrachannels=extrachannels;


  printf("Open file '%s': ", path);
  ambix=ambix_open(path, AMBIX_WRITE, &info);
  if(!ambix) {
    printf("failed\n");
    return;
  } else
    printf("OK\n");

  printf("Frames\t: %d\n", info.frames);

  printf("Samplerate\t: %f\n", info.samplerate);

  printf("Sampleformat\t: %d (", info.sampleformat);
  switch(info.sampleformat) {
  case(AMBIX_SAMPLEFORMAT_NONE): printf("NONE"); break;
  case(AMBIX_SAMPLEFORMAT_PCM16): printf("PCM16"); break;
  case(AMBIX_SAMPLEFORMAT_PCM24): printf("PCM24"); break;
  case(AMBIX_SAMPLEFORMAT_PCM32): printf("PCM32"); break;
  case(AMBIX_SAMPLEFORMAT_FLOAT32): printf("FLOAT32"); break;
  default: printf("**unknown**");
  }
  printf(")\n");

  printf("ambiXformat\t: %d (", info.fileformat);
  switch(info.fileformat) {
  case(AMBIX_NONE): printf("NONE"); break;
  case(AMBIX_BASIC): printf("BASIC"); break;
  case(AMBIX_EXTENDED): printf("EXTENDED"); break;
  default: printf("**unknown** 0x%04X", info.fileformat);
  }
  printf(")\n");

  printf("Ambisonics channels\t: %d\n", info.ambichannels);
  printf("Non-Ambisonics channels\t: %d\n", info.extrachannels);



  /* no write some data... */
  do {
    uint64_t res;
    float32_t ambidata[64*ambichannels];
    float32_t otherdata[64*extrachannels];
    while(frames>64) {
      res=ambix_writef_float32(ambix, ambidata, otherdata, 64);
      frames-=64;
    }
    if(frames>0)
      res=ambix_writef_float32(ambix, ambidata, otherdata, frames);

  } while(0);



  printf("Close file '%s': ", path);
  if(AMBIX_ERR_SUCCESS!=ambix_close(ambix))
    printf("failed\n");
  else
    printf("OK\n");
}

void createfile_extended(const char*path, uint32_t ambichannels, uint32_t extrachannels, uint64_t frames) {
  ambix_info_t info;
  ambix_t*ambix;
  const ambix_matrix_t*matrix;
  ambix_matrix_t adaptmatrix;
  ambix_err_t err;

  memset(&info, 0, sizeof(info));
  info.frames=frames;
  info.samplerate=44100;
  info.sampleformat=AMBIX_SAMPLEFORMAT_FLOAT32;
  info.fileformat=AMBIX_EXTENDED;
  info.ambichannels=ambichannels;
  info.extrachannels=extrachannels;


  printf("Open file '%s': ", path);
  ambix=ambix_open(path, AMBIX_WRITE, &info);
  if(!ambix) {
    printf("failed\n");
    return;
  } else
    printf("OK\n");

  printf("Frames\t: %d\n", info.frames);

  printf("Samplerate\t: %f\n", info.samplerate);

  printf("Sampleformat\t: %d (", info.sampleformat);
  switch(info.sampleformat) {
  case(AMBIX_SAMPLEFORMAT_NONE): printf("NONE"); break;
  case(AMBIX_SAMPLEFORMAT_PCM16): printf("PCM16"); break;
  case(AMBIX_SAMPLEFORMAT_PCM24): printf("PCM24"); break;
  case(AMBIX_SAMPLEFORMAT_PCM32): printf("PCM32"); break;
  case(AMBIX_SAMPLEFORMAT_FLOAT32): printf("FLOAT32"); break;
  default: printf("**unknown**");
  }
  printf(")\n");

  printf("ambiXformat\t: %d (", info.fileformat);
  switch(info.fileformat) {
  case(AMBIX_NONE): printf("NONE"); break;
  case(AMBIX_BASIC): printf("BASIC"); break;
  case(AMBIX_EXTENDED): printf("EXTENDED"); break;
  default: printf("**unknown** 0x%04X", info.fileformat);
  }
  printf(")\n");

  printf("Ambisonics channels\t: %d\n", info.ambichannels);
  printf("Non-Ambisonics channels\t: %d\n", info.extrachannels);

  memset(&adaptmatrix, 0, sizeof(adaptmatrix));
  ambix_matrix_init(25, ambichannels,&adaptmatrix);

  do {
    uint32_t r, c, rows=adaptmatrix.rows, cols=adaptmatrix.cols;
    float32_t**data=adaptmatrix.data;
    for(r=0; r<rows; r++) {
      for(c=0; c<cols; c++) {
        data[r][c]=(float32_t)r+((float32_t)c)/25.;
      }
    }
  } while(0);

  err=ambix_set_adaptormatrix(ambix, &adaptmatrix);
  if(err!=AMBIX_ERR_SUCCESS) {
    printf("failed setting adaptor matrix\n");
  }

  /* no write some data... */
  printf("Writing %d frames\n", frames);
  do {
    const int blocksize=64;
    uint64_t res;
    float32_t ambidata[blocksize*ambichannels];
    float32_t otherdata[blocksize*extrachannels];
    while(frames>blocksize) {
      res=ambix_writef_float32(ambix, ambidata, otherdata, blocksize);
      if(res!=blocksize)
        printf("write returned %d!=%d\n", res, blocksize);
      frames-=blocksize;
    }
    if(frames>0) {
      res=ambix_writef_float32(ambix, ambidata, otherdata, frames);
      if(res!=frames)
        printf("write returned %d!=%d\n", res, frames);
    }
  } while(0);



  printf("Close file '%s': ", path);
  if(AMBIX_ERR_SUCCESS!=ambix_close(ambix))
    printf("failed\n");
  else
    printf("OK\n");

  ambix_matrix_deinit(&adaptmatrix);
}



int main(int argc, char**argv) {
  if(argc>1) {
    createfile_basic(argv[1], 9, 3, 44100);
    if(argc>2)
      createfile_extended(argv[2], 9, 3, 44100);
  }
  else {
    fprintf(stderr, "usage: %s <ambixfilename>\n", argv[0]);
  }

  return 0;
}
