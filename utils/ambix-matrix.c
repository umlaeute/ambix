/* ambix-matrix -  display ambix matrices              -*- c -*-

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_version(const char*name);
void print_usage(const char*name);

typedef struct {
  ambix_matrixtype_t typ;
  const char*name;
} matrix_map_t;

#if 0
static matrix_map_t matrixmap[] = {
  AMBIX_MATRIX_ZERO, "MATRIX_ZERO",
  AMBIX_MATRIX_ONE, "MATRIX_ONE",
  AMBIX_MATRIX_AMBIX, "MATRIX_AMBIX",
  AMBIX_MATRIX_N3D, "MATRIX_N3D",
  AMBIX_MATRIX_SID, "MATRIX_SID",
  AMBIX_MATRIX_FUMA, "MATRIX_FUMA",
  AMBIX_MATRIX_TO_AMBIX, "MATRIX_TO_AMBIX",
  AMBIX_MATRIX_TO_N3D, "MATRIX_TO_N3D",
  AMBIX_MATRIX_TO_SID, "MATRIX_TO_SID",
  AMBIX_MATRIX_TO_FUMA, "MATRIX_TO_FUMA",
};
#endif


static void printmatrix(const ambix_matrix_t*mtx) {
  printf("matrix %p", mtx);
  if(mtx) {
    float32_t**data=mtx->data;
    uint32_t r, c;
    printf(" [%dx%d] = %p\n", mtx->rows, mtx->cols, mtx->data);
    for(r=0; r<mtx->rows; r++) {
      for(c=0; c<mtx->cols; c++) {
        printf("%08f ", data[r][c]);
      }
      printf("\n");
    }
  }
  printf("\n");
}

static void print_matrix(const char*name, ambix_matrixtype_t typ, uint32_t rows, uint32_t cols) {
  ambix_matrix_t*mtx=ambix_matrix_init(rows, cols, NULL);
  if(mtx) {
    ambix_matrix_t*result=ambix_matrix_fill(mtx, typ);
    if(result) {
      printf("%s\t%d\n", name, (int)typ);
      printmatrix(result);
      if(result!=mtx)
        ambix_matrix_destroy(result);
    } else {
      printf("couldn't create matrix [%dx%d] of type %s[%d]\n", rows, cols, name, typ);
    }

    ambix_matrix_destroy(mtx);
  }
}


int main(int argc, char**argv) {
  if(argc>1) {
    if((!strcmp(argv[1], "-V")) || (!strcmp(argv[1], "--version")))
      print_version(argv[0]);
    if((!strcmp(argv[1], "-h")) || (!strcmp(argv[1], "--help")))
      print_usage(argv[0]);
  }

  print_matrix("FuMa []"   , AMBIX_MATRIX_FUMA,  1,  1);
  print_matrix("FuMa [h]"  , AMBIX_MATRIX_FUMA,  4,  3);
  print_matrix("FuMa [f]"  , AMBIX_MATRIX_FUMA,  4,  4);
  print_matrix("FuMa [hh]" , AMBIX_MATRIX_FUMA,  9,  5);
  print_matrix("FuMa [fh]" , AMBIX_MATRIX_FUMA,  9,  6);
  print_matrix("FuMa [ff]" , AMBIX_MATRIX_FUMA,  9,  9);
  print_matrix("FuMa [hhh]", AMBIX_MATRIX_FUMA, 16,  7);
  print_matrix("FuMa [fhh]", AMBIX_MATRIX_FUMA, 16,  8);
  print_matrix("FuMa [ffh]", AMBIX_MATRIX_FUMA, 16, 11);
  print_matrix("FuMa [fff]", AMBIX_MATRIX_FUMA, 16, 16);



  print_matrix("MaFu []"   , AMBIX_MATRIX_TO_FUMA,  1,  1);
  print_matrix("MaFu [h]"  , AMBIX_MATRIX_TO_FUMA,  3,  4);
  print_matrix("MaFu [f]"  , AMBIX_MATRIX_TO_FUMA,  4,  4);
  print_matrix("MaFu [hh]" , AMBIX_MATRIX_TO_FUMA,  5,  9);
  print_matrix("MaFu [fh]" , AMBIX_MATRIX_TO_FUMA,  6,  9);
  print_matrix("MaFu [ff]" , AMBIX_MATRIX_TO_FUMA,  9,  9);
  print_matrix("MaFu [hhh]", AMBIX_MATRIX_TO_FUMA,  7, 16);
  print_matrix("MaFu [fhh]", AMBIX_MATRIX_TO_FUMA,  8, 16);
  print_matrix("MaFu [ffh]", AMBIX_MATRIX_TO_FUMA, 11, 16);
  print_matrix("MaFu [fff]", AMBIX_MATRIX_TO_FUMA, 16, 16);


  print_matrix("zero [16]", AMBIX_MATRIX_ZERO, 16, 16);
  print_matrix("one [16]", AMBIX_MATRIX_ONE, 16, 16);
  print_matrix("identity [16]", AMBIX_MATRIX_IDENTITY, 16, 16);

  print_matrix("SID [16]", AMBIX_MATRIX_SID, 16, 16);
  print_matrix("->SID [16]", AMBIX_MATRIX_TO_SID, 16, 16);

  print_matrix("N3D [16]", AMBIX_MATRIX_N3D, 16, 16);
  print_matrix("->N3D [16]", AMBIX_MATRIX_TO_N3D, 16, 16);

  return 0;
}

void print_usage(const char*name) {
  printf("\n");
  printf("Usage: %s\n", name);
  printf("Print some standard matrices when dealing with ambisonics\n");
  printf("(this may be of limited use when not debugging libambix).\n");

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
