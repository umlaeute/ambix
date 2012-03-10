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

void createfile_simple(const char*path, uint32_t ambichannels, uint32_t otherchannels, uint64_t frames) {
  ambixinfo_t info;
  ambix_t*ambix;
  const ambixmatrix_t*matrix;

  memset(&info, 0, sizeof(info));
  info.frames=frames;
  info.samplerate=44100;
  info.sampleformat=AMBIX_SAMPLEFORMAT_FLOAT32;
  info.ambixfileformat=AMBIX_SIMPLE;
  info.ambichannels=ambichannels;
  info.otherchannels=otherchannels;


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

  printf("ambiXformat\t: %d (", info.ambixfileformat);
  switch(info.ambixfileformat) {
  case(AMBIX_NONE): printf("NONE"); break;
  case(AMBIX_SIMPLE): printf("SIMPLE"); break;
  case(AMBIX_EXTENDED): printf("EXTENDED"); break;
  default: printf("**unknown** 0x%04X", info.ambixfileformat);
  }
  printf(")\n");

  printf("Ambisonics channels\t: %d\n", info.ambichannels); 
  printf("Non-Ambisonics channels\t: %d\n", info.otherchannels);



  /* no write some data... */
  do {
    uint64_t res;
    float32_t ambidata[64*ambichannels];
    float32_t otherdata[64*otherchannels];
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

int main(int argc, char**argv) {
  if(argc>1)
    createfile_simple(argv[1], 9, 2, 44100);
  else {
    fprintf(stderr, "usage: %s <ambixfilename>\n", argv[0]);
  }

  return 0;
}
