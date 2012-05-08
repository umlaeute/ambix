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

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include <math.h>
#include <string.h>

ambix_matrix_t*
_matrix_fuma2ambix(uint32_t cols) {
  ambix_matrix_t*result=NULL, *weightm=NULL, *routem=NULL, *reducem=NULL;
  uint32_t rows=0;
  int32_t r, c;
  const float32_t sqrt2=sqrt(2.);          // 1.414

  const float32_t sqrt4_3=2./sqrt(3.);     // 1.154
  const float32_t sqrt45_32=sqrt(45./32.); // 1.185
  const float32_t sqrt9_5=3./sqrt(5.);     // 1.341
  const float32_t sqrt8_5=sqrt(8./5.);     // 1.264

  const float32_t sqrt3_4  = sqrt(3.)/2.;      // 0.86603
  const float32_t sqrt5_8  = sqrt(5./2.)/2.;   // 0.79057
  const float32_t sqrt32_45= 4.*sqrt(2./5.)/3.;// 0.84327
  const float32_t sqrt5_9  = sqrt(5.)/3.;      // 0.74536


  float32_t order[]={
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
  
  int i;
  float32_t*reducer=NULL;

  switch(cols) {
  case  3: /* h   = 1st order 2-D */
    reducer=(float32_t[]) {0, 1, 2}; // WXY
    rows=4;
    break;
  case  4: /* f   = 1st order 3-D */
    reducer=(float32_t[]) {0, 1, 2, 3}; // WXYZ
    rows=4;
    break;
  case  5: /* hh  = 2nd order 2-D */
    reducer=(float32_t[]) {0, 1, 2, 4, 5}; // WXYRS
    rows=9;
    break;
  case  6: /* fh  = 2nd order 2-D + 1st order 3-D (formerly called 2.5 order) */
    reducer=(float32_t[]) {0, 1, 2, 3, 4, 5}; // WXYZRS
    rows=9;
    break;
  case  7: /* hhh = 3rd order 2-D */
    reducer=(float32_t[]) {0, 1, 2, 4, 5, 14, 15}; // WXYRSPQ
    rows=16;
    break;
  case  8: /* fhh = 3rd order 2-D + 1st order 3-D */
    reducer=(float32_t[]) {0, 1, 2, 3, 4, 5, 14, 15}; // WXYZRSPQ
    rows=16;
    break;
  case  9: /* ff  = 2nd order 3-D */
    reducer=(float32_t[]) {0, 1, 2, 3, 4, 5, 6, 7, 8}; // WXYZRSTUV
    rows=9;
    break;
  case 11: /* ffh = 3rd order 2-D + 2nd order 3-D */
    reducer=(float32_t[]) {0, 1, 2, 3, 4, 5, 6, 7, 8, 14, 15}; // WXYZRSTUVPQ
    rows=16;
    break;
  case 16: /* fff = 3rd order 3-D */
    reducer=(float32_t[]) {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}; // WXYZRSTUVKLMNOPQ
    rows=16;
    break;
  default:break;
  }

  if(reducer) {
    ambix_matrix_t*expand_m=NULL;
    ambix_matrix_t*weight_m=NULL;
    ambix_matrix_t*order_m =NULL;
    ambix_matrix_t*reduce_m=NULL;
    ambix_matrix_t*temp_m=NULL;
    ambix_matrix_t*result_m=NULL;

    reduce_m=ambix_matrix_init(16, cols, reduce_m);
    if(!_matrix_permutate(reduce_m, reducer, 1)) {

    }
    //printf("reducer");_ambix_print_matrix(reduce_m);

    expand_m=ambix_matrix_init(rows, 16, expand_m);
    expand_m=ambix_matrix_fill(expand_m, AMBIX_MATRIX_IDENTITY);

    weight_m=_matrix_diag  (weight_m, weights, sizeof(weights)/sizeof(*weights));
    order_m =_matrix_router(order_m , order, sizeof(order)/sizeof(*order), 1);

    temp_m=ambix_matrix_multiply(weight_m, order_m, temp_m);

    //printf("weight");_ambix_print_matrix(weight_m);
    //printf("order"); _ambix_print_matrix(order_m);

    ambix_matrix_destroy(weight_m);weight_m=NULL;
    ambix_matrix_destroy(order_m); order_m=NULL;

    result_m=ambix_matrix_multiply(temp_m, reduce_m, result_m);
    temp_m=ambix_matrix_multiply(expand_m, result_m, temp_m);

    ambix_matrix_destroy(reduce_m); reduce_m=NULL;
    ambix_matrix_destroy(result_m); result_m=NULL;

    //printf("#final#");_ambix_print_matrix(temp_m);

    return temp_m;
  }
  return NULL;
}
