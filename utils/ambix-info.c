/* ambix_info -  display info about an ambix file              -*- c -*-

   Copyright © 2012 IOhannes m zmölnig <zmoelnig@iem.at>.
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
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "ambix/ambix.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void print_usage(const char*name);
void print_version(const char*name);

void print_markers(ambix_t*ambix)
{
  uint32_t num_markers = ambix_get_num_markers(ambix);
  if (num_markers) {
    ambix_marker_t *marker = NULL;
    uint32_t i;
    for (i=0; i < num_markers; i++) {
      marker = ambix_get_marker(ambix, i);
      if (marker)
        printf("  Marker %d: name: %s position: %f \n", i, marker->name, marker->position);
    }
  }
}
void print_regions(ambix_t*ambix)
{
  uint32_t num_regions = ambix_get_num_regions(ambix);
  if (num_regions) {
    ambix_region_t *region = NULL;
    uint32_t i;
    for (i=0; i < num_regions; i++) {
      region = ambix_get_region(ambix, i);
      printf("  Region %d: name: %s start_position: %f end_position: %f \n", i, region->name, region->start_position, region->end_position);
    }
  }
}

void printinfo(const char*path) {
  ambix_info_t info;
  ambix_t*ambix;
  const ambix_matrix_t*matrix;
  memset(&info, 0, sizeof(info));

  printf("Open file '%s': ", path);

  ambix=ambix_open(path, AMBIX_READ, &info);
  if(!ambix) {
    printf("failed\n");
    return;
  } else
    printf("OK\n");

  printf("Frames\t: %d\n", (int)(info.frames));

  printf("Samplerate\t: %f\n", info.samplerate);

  printf("Sampleformat\t: %d (", info.sampleformat);
  switch(info.sampleformat) {
  case(AMBIX_SAMPLEFORMAT_NONE): printf("NONE"); break;
  case(AMBIX_SAMPLEFORMAT_PCM16): printf("PCM16"); break;
  case(AMBIX_SAMPLEFORMAT_PCM24): printf("PCM24"); break;
  case(AMBIX_SAMPLEFORMAT_PCM32): printf("PCM32"); break;
  case(AMBIX_SAMPLEFORMAT_FLOAT32): printf("FLOAT32"); break;
  case(AMBIX_SAMPLEFORMAT_FLOAT64): printf("FLOAT64"); break;
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



  matrix=ambix_get_adaptormatrix(ambix);
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

  printf("Number of Markers\t: %d\n", ambix_get_num_markers(ambix));
  print_markers(ambix);
  printf("Number of Regions\t: %d\n", ambix_get_num_regions(ambix));
  print_regions(ambix);

  printf("Close file '%s': ", path);
  if(AMBIX_ERR_SUCCESS!=ambix_close(ambix))
    printf("failed\n");
  else
    printf("OK\n");
}

int main(int argc, char**argv) {
  if(argc>1) {
    int i;
    if((!strcmp(argv[1], "-V")) || (!strcmp(argv[1], "--version")))
      print_version(argv[0]);
    if((!strcmp(argv[1], "-h")) || (!strcmp(argv[1], "--help")))
      print_usage(argv[0]);

    for(i=1; i<argc; i++) {
      printinfo(argv[i]);
      printf("\n");
    }

  }
  else {
    print_usage(argv[0]);
  }

  return 0;
}


void print_usage(const char*name) {
  printf("\n");
  printf("Usage: %s file1 [file2...]\n", name);
  printf("Print information on ambix files\n");

  printf("\n");
  printf("Options:\n");
  printf("  -h, --help                       Print this help\n");
  printf("  -V, --version                    Version information\n");
  printf("\n");

#ifdef PACKAGE_BUGREPORT
  printf("Report bugs to: %s\n\n", PACKAGE_BUGREPORT);
#endif
#ifdef PACKAGE_URL
  printf("Home page: %s\n", PACKAGE_URL);
#endif

  exit(1);
}
void print_version(const char*name) {
#ifdef PACKAGE_VERSION
  printf("%s %s\n", name, PACKAGE_VERSION);
#endif
  printf("\n");
  printf("Copyright (C) 2012 Institute of Electronic Music and Acoustics (IEM), University of Music and Dramatic Arts (KUG), Graz, Austria.\n");
  printf("\n");
  printf("License LGPLv2.1: GNU Lesser GPL version 2.1 or later <http://gnu.org/licenses/lgpl.html>\n");
  printf("This is free software: you are free to change and redistribute it.\n");
  printf("There is NO WARRANTY, to the extent permitted by law.\n");
  printf("\n");
  printf("Written by IOhannes m zmoelnig <zmoelnig@iem.at>\n");
  exit(1);
}
