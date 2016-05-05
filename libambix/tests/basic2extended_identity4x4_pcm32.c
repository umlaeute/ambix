/* extended_float32_1024 - test ambix extended (PCM32, blocksize 1024)

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

#include "common.h"

int check_create_b2e(const char*path, ambix_sampleformat_t format,
		     ambix_matrix_t*matrix, uint32_t extrachannels,
		     uint32_t chunksize, float32_t eps);
int test_defaultmatrix(const char*name, uint32_t rows, uint32_t cols, ambix_matrixtype_t mtyp,
		       uint32_t xtrachannels, uint32_t chunksize, float32_t eps) {
  int result=0;
  ambix_matrix_t*mtx=0;
  STARTTEST("%s\n", name);
  mtx=ambix_matrix_init(rows,cols,mtx);
  if(!mtx)return 1;
  ambix_matrix_fill(mtx, mtyp);
  result=check_create_b2e("test2-b2e-float32.caf", AMBIX_SAMPLEFORMAT_PCM32,
			  mtx,xtrachannels,
			  chunksize, eps);
  ambix_matrix_destroy(mtx);
  return result;
}

int main(int argc, char**argv) {
  int err=0;
  err+=test_defaultmatrix("IDENTITY", 4, 4, AMBIX_MATRIX_IDENTITY, 0, 1024, 1e-9);
  pass();
  return 0;
}
