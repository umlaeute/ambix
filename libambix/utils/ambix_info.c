/* ambix_info -  display info about an ambix file              -*- c -*-

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


void printinfo(const char*path) {
  ambixinfo_t info;
  ambix_t*ambix;
  const ambixmatrix_t*matrix;


  printf("Open file '%s': ", path);

  ambix=ambix_open(path, AMBIX_READ, &info);
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



  matrix=ambix_getAdaptorMatrix(ambix);
  printf("Reconstruction matrix\t: ");
  if(!matrix) {
    printf("**none**");
  } else {
    uint32_t r, c;
    printf("[%dx%d]\n", matrix->rows, matrix->cols);
    for(r=0; r<matrix->rows; r++) {
      printf("\t");
      for(c=0; c<matrix->cols; c++) {
        printf("%.6f ", matrix->data[r][c]);
      }
      printf("\n");
    }
  }
  printf("\n");

  printf("Close file '%s': ", path);
  if(AMBIX_ERR_SUCCESS!=ambix_close(ambix))
    printf("failed\n");
  else 
    printf("OK\n");
}

int main(int argc, char**argv) {
  if(argc>1)
    printinfo(argv[1]);
  else {
    fprintf(stderr, "usage: %s <ambixfilename>\n", argv[0]);
  }

  return 0;
}
