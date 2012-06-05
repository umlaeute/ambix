/* adaptor_fuma.c -  adaptors to/from FUrse-MAlham sets              -*- c -*-

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
  const float32_t sqrt2    = (float32_t)(sqrt(2.));         // 1.414
  const float32_t sqrt3_4  = (float32_t)(sqrt(3.)/2.);      // 0.86603
  const float32_t sqrt5_8  = (float32_t)(sqrt(5./2.)/2.);   // 0.79057
  const float32_t sqrt32_45= (float32_t)(4.*sqrt(2./5.)/3.);// 0.84327
  const float32_t sqrt5_9  = (float32_t)(sqrt(5.)/3.);      // 0.74536

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

static ambix_matrix_t*
ambix2fuma_weightorder(void) {
  const float32_t sqrt1_2  = (float32_t)(1./sqrt(2.));       // 0.707
  const float32_t sqrt4_3  = (float32_t)(2./sqrt(3.));      // 1.1547
  const float32_t sqrt8_5  = (float32_t)(2.*sqrt(2./5.));   // 1.2649
  const float32_t sqrt45_32= (float32_t)(3.*sqrt(5./2.)/4.);// 1.1856
  const float32_t sqrt9_5  = (float32_t)(3./sqrt(5.));      // 1.3416

  static float32_t order[]={
    0,
    2, 3, 1,
    8, 6, 4, 5, 7,
    15, 13, 11,  9, 10, 12, 14,
  };
  float32_t weights[]={
    sqrt1_2,
    -1, 1, -1,
    sqrt4_3, -sqrt4_3, 1, -sqrt4_3, sqrt4_3,
    -sqrt8_5, sqrt9_5, -sqrt45_32, 1, -sqrt45_32, sqrt9_5, -sqrt8_5,
  };

  ambix_matrix_t*result_m=NULL;
  ambix_matrix_t*weight_m=NULL;
  ambix_matrix_t*order_m =NULL;

  weight_m=_matrix_diag  (weight_m, weights, sizeof(weights)/sizeof(*weights));
  order_m =_matrix_router(order_m , order, sizeof(order)/sizeof(*order), 0);

  result_m=ambix_matrix_multiply(order_m, weight_m, result_m);

  ambix_matrix_destroy(weight_m);weight_m=NULL;
  ambix_matrix_destroy(order_m); order_m=NULL;

  return result_m;
}


static ambix_matrix_t*
_matrix_multiply3(ambix_matrix_t*mtx1, 
                  ambix_matrix_t*mtx2,
                  ambix_matrix_t*mtx3,
                  ambix_matrix_t*result) {
  ambix_matrix_t*tmp=NULL;

  tmp=ambix_matrix_multiply(mtx1, mtx2, tmp);
  result=ambix_matrix_multiply(tmp, mtx3, result);

  ambix_matrix_destroy(tmp);
  return result;
}

ambix_matrix_t*
_matrix_ambix2fuma(uint32_t cols) {
  uint32_t rows=0;
#if 1
  float32_t reducer_v[17][16]={
    {0}, /* NULL */
    {0}, // W
    {0}, /* NULL */
    {0, 1, 2}, // WXY
    {0, 1, 2, 3}, // WXYZ
    {0, 1, 2, 4, 5}, // WXYRS
    {0, 1, 2, 3, 4, 5}, // WXYZRS
    {0, 1, 2, 4, 5, 14, 15}, // WXYRSPQ
    {0, 1, 2, 3, 4, 5, 14, 15}, // WXYZRSPQ
    {0, 1, 2, 3, 4, 5, 6, 7, 8}, // WXYZRSTUV
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* NULL */
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 14, 15}, // WXYZRSTUVPQ
    {0}, /* NULL */
    {0}, /* NULL */
    {0}, /* NULL */
    {0}, /* NULL */
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, // WXYZRSTUVKLMNOPQ
  };
#else
  float32_t * reducer_v[]={
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
#endif
  static const uint32_t rows_v[]={
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

  if(rows_v[cols]) {
    ambix_matrix_t*weightorder_m=NULL;

    ambix_matrix_t*expand_m=NULL;
    ambix_matrix_t*reduce_m=NULL;
    ambix_matrix_t*final_m=NULL;

    expand_m=ambix_matrix_init(16, rows_v[cols], expand_m);
    expand_m=ambix_matrix_fill(expand_m, AMBIX_MATRIX_IDENTITY);

    if(NULL==weightorder_m)
      weightorder_m=ambix2fuma_weightorder();

    reduce_m=ambix_matrix_init(cols, 16, reduce_m);
    if(!_matrix_permutate(reduce_m, reducer_v[cols], 0)) {
      return NULL;
    }

    final_m=_matrix_multiply3(reduce_m, weightorder_m, expand_m, final_m);

    ambix_matrix_destroy(expand_m); expand_m=NULL;
    ambix_matrix_destroy(reduce_m); reduce_m=NULL;
    ambix_matrix_destroy(weightorder_m); weightorder_m=NULL;

    return final_m;
  }
  return NULL;
}


ambix_matrix_t*
_matrix_fuma2ambix(uint32_t rows) {
  uint32_t cols=0;

#if 1
  float32_t reducer_v[17][16]={
    {0}, /* NULL */
    {0}, // W
    {0}, /* NULL */
    {0, 1, 2}, // WXY
    {0, 1, 2, 3}, // WXYZ
    {0, 1, 2, 4, 5}, // WXYRS
    {0, 1, 2, 3, 4, 5}, // WXYZRS
    {0, 1, 2, 4, 5, 14, 15}, // WXYRSPQ
    {0, 1, 2, 3, 4, 5, 14, 15}, // WXYZRSPQ
    {0, 1, 2, 3, 4, 5, 6, 7, 8}, // WXYZRSTUV
    {0}, /* NULL */
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 14, 15}, // WXYZRSTUVPQ
    {0}, /* NULL */
    {0}, /* NULL */
    {0}, /* NULL */
    {0}, /* NULL */
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, // WXYZRSTUVKLMNOPQ
  };
#else
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
#endif

  static const uint32_t cols_v[]={
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

  if(rows<0 || rows > 16)
    return NULL;

  if(cols_v[rows]) {
    ambix_matrix_t*weightorder_m=NULL;

    ambix_matrix_t*expand_m=NULL;
    ambix_matrix_t*reduce_m=NULL;
    ambix_matrix_t*final_m=NULL;

    expand_m=ambix_matrix_init(cols_v[rows], 16, expand_m);
    expand_m=ambix_matrix_fill(expand_m, AMBIX_MATRIX_IDENTITY);

    if(NULL==weightorder_m)
      weightorder_m=fuma2ambix_weightorder();

    reduce_m=ambix_matrix_init(16, rows, reduce_m);
    if(!_matrix_permutate(reduce_m, reducer_v[rows], 1)) {
      return NULL;
    }

    final_m=_matrix_multiply3(expand_m, weightorder_m, reduce_m, final_m);

    ambix_matrix_destroy(expand_m); expand_m=NULL;
    ambix_matrix_destroy(reduce_m); reduce_m=NULL;
    ambix_matrix_destroy(weightorder_m); weightorder_m=NULL;

    return final_m;
  }
  return NULL;
}
