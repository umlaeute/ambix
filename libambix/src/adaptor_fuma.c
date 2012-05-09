/* adaptor_fuma.c -  adaptors to/from FUrse-MAlham sets              -*- c -*-

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

/*
#chan|fuma|   type     | xy | z | channels         | done
  3 	h 	  horizontal 	  1 	0 	WXY                 X
  4 	f 	  full-sphere 	1 	1 	WXYZ                X
  5 	hh 	  horizontal   	2 	0 	WXYRS
  6 	fh 	  mixed-order 	2 	1 	WXYZRS
  9 	ff 	  full-sphere 	2 	2 	WXYZRSTUV           X
  7 	hhh 	horizontal 	  3 	0 	WXYRSPQ
  8 	fhh 	mixed-order 	3 	1 	WXYZRSPQ
  11 	ffh 	mixed-order 	3 	2 	WXYZRSTUVPQ
  16 	fff 	full-sphere 	3 	3 	WXYZRSTUVKLMNOPQ    X
*/

#include "private.h"

#include <math.h>
#include <string.h>

static ambix_matrix_t*
fuma2ambix_weightorder(void) {
  const float32_t sqrt2=sqrt(2.);          // 1.414
  const float32_t sqrt3_4  = sqrt(3.)/2.;      // 0.86603
  const float32_t sqrt5_8  = sqrt(5./2.)/2.;   // 0.79057
  const float32_t sqrt32_45= 4.*sqrt(2./5.)/3.;// 0.84327
  const float32_t sqrt5_9  = sqrt(5.)/3.;      // 0.74536

  static float32_t order[]={
    0,
    2, 3, 1,
    8, 6, 4, 5, 7,
    15, 13, 11,  9, 10, 12, 14,
  };
  float32_t weights[]={
    sqrt2,
    -1, 1, -1,
    sqrt3_4, -sqrt3_4, 1, -sqrt3_4, sqrt3_4,
    -sqrt5_8, sqrt5_9, -sqrt32_45, 1, -sqrt32_45, sqrt5_9, -sqrt5_8,
  };

  ambix_matrix_t*result_m=NULL;
  ambix_matrix_t*weight_m=NULL;
  ambix_matrix_t*order_m =NULL;

  weight_m=_matrix_diag  (weight_m, weights, sizeof(weights)/sizeof(*weights));
  order_m =_matrix_router(order_m , order, sizeof(order)/sizeof(*order), 1);

  result_m=ambix_matrix_multiply(weight_m, order_m, result_m);

  ambix_matrix_destroy(weight_m);weight_m=NULL;
  ambix_matrix_destroy(order_m); order_m=NULL;

  return result_m;
}

ambix_matrix_t*
_matrix_fuma2ambix(uint32_t cols) {
  uint32_t rows=0;
  
  int i;
  float32_t*reducer=NULL;

  float32_t*reducer_v[]={
    NULL,
    (float32_t[]) {0}, // W
    NULL,
    (float32_t[]) {0, 1, 2}, // WXY
    (float32_t[]) {0, 1, 2, 3}, // WXYZ
    (float32_t[]) {0, 1, 2, 4, 5}, // WXYRS
    (float32_t[]) {0, 1, 2, 3, 4, 5}, // WXYZRS
    (float32_t[]) {0, 1, 2, 4, 5, 14, 15}, // WXYRSPQ
    (float32_t[]) {0, 1, 2, 3, 4, 5, 14, 15}, // WXYZRSPQ
    (float32_t[]) {0, 1, 2, 3, 4, 5, 6, 7, 8}, // WXYZRSTUV
    NULL,
    (float32_t[]) {0, 1, 2, 3, 4, 5, 6, 7, 8, 14, 15}, // WXYZRSTUVPQ
    NULL,
    NULL,
    NULL,
    NULL,
    (float32_t[]) {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, // WXYZRSTUVKLMNOPQ
  };

  static uint32_t rows_v[]={
    0,
    1, // W
    0,
    4, // WXY
    4, // WXYZ
    9, // WXYRS
    9, // WXYZRS
    16,// WXYRSPQ
    16,// WXYZRSPQ
    9, // WXYZRSTUV
    0,
    16,// WXYZRSTUVPQ
    0,
    0,
    0,
    0,
    16,// WXYZRSTUVKLMNOPQ
  };

  if(cols<0 || cols > 16)
    return NULL;

  reducer= reducer_v[cols];
  rows   = rows_v   [cols];

  if(reducer) {
    static ambix_matrix_t*weightorder_m=NULL;

    ambix_matrix_t*expand_m=NULL;
    ambix_matrix_t*reduce_m=NULL;
    ambix_matrix_t*result_m=NULL;
    ambix_matrix_t*final_m=NULL;
    if(NULL==weightorder_m)
      weightorder_m=fuma2ambix_weightorder();

    reduce_m=ambix_matrix_init(16, cols, reduce_m);
    if(!_matrix_permutate(reduce_m, reducer, 1)) {
      return NULL;
    }

    expand_m=ambix_matrix_init(rows, 16, expand_m);
    expand_m=ambix_matrix_fill(expand_m, AMBIX_MATRIX_IDENTITY);

    result_m=ambix_matrix_multiply(weightorder_m, reduce_m, result_m);
    final_m=ambix_matrix_multiply(expand_m, result_m, final_m);

    ambix_matrix_destroy(reduce_m); reduce_m=NULL;
    ambix_matrix_destroy(result_m); result_m=NULL;

    return final_m;
  }
  return NULL;
}
