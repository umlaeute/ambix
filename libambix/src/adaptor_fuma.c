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

ambix_matrix_t*
_matrix_fuma2ambix(uint32_t cols) {
  ambix_matrix_t*result=NULL, *weightm=NULL, *routem=NULL, *reducem=NULL;
  uint32_t rows=0;
  int32_t r, c;
  const float32_t sqrt2=sqrt(2);

#warning fix matrix_type for FuMa2ambix

  switch(cols) {
  case  3: /* h   = 1st order 2-D */
    if(1) {
      float32_t weights[]={1.4142f, 1.f, 1.f, 1.f};
      float32_t ordering[4];
      rows=sizeof(ordering)/sizeof(*ordering);
      weights[0]=sqrt2;
      _matrix_sid2acn(ordering, sizeof(ordering)/sizeof(*ordering));

      weightm=_matrix_diag(NULL, weights, sizeof(weights)/sizeof(*weights));
      routem =_matrix_router(NULL, ordering, sizeof(ordering)/sizeof(*ordering), 0);
      reducem=ambix_matrix_init(rows, cols, NULL);
      reducem=ambix_matrix_fill(reducem, AMBIX_MATRIX_IDENTITY);
    }
    break;
  case  4: /* f   = 1st order 3-D */
    if(1) {
      float32_t weights[]={1.4142f, 1.f, 1.f, 1.f};
      float32_t ordering[4];
      rows=sizeof(ordering)/sizeof(*ordering);
      weights[0]=sqrt2;
      _matrix_sid2acn(ordering, sizeof(ordering)/sizeof(*ordering));
      
      weightm=_matrix_diag(NULL, weights, sizeof(weights)/sizeof(*weights));
      routem =_matrix_router(NULL, ordering, sizeof(ordering)/sizeof(*ordering), 0);
      reducem=ambix_matrix_init(rows, cols, NULL);
      reducem=ambix_matrix_fill(reducem, AMBIX_MATRIX_IDENTITY);
    }
    break;
  case  5: /* hh  = 2nd order 2-D */
    if(9==rows) {
    }
    break;
  case  6: /* fh  = 2nd order 2-D + 1st order 3-D (formerly called 2.5 order) */
    if(9==rows) {
    }
    break;
  case  7: /* hhh = 3rd order 2-D */
    if(16==rows) {
    }
    break;
  case  8: /* fhh = 3rd order 2-D + 1st order 3-D */
    if(16==rows) {
    }
    break;
  case  9: /* ff  = 2nd order 3-D */
    if(1) {
      float32_t ordering[9];
      rows=sizeof(ordering)/sizeof(*ordering);
      _matrix_sid2acn(ordering, sizeof(ordering)/sizeof(*ordering));
      routem =_matrix_router(NULL, ordering, sizeof(ordering)/sizeof(*ordering), 0);
    }
    break;
  case 11: /* ffh = 3rd order 2-D + 2nd order 3-D */
    if(16==rows) {
    }
    break;
  case 16: /* fff = 3rd order 3-D */
    if(1) {
      float32_t ordering[16];
      rows=sizeof(ordering)/sizeof(*ordering);
      _matrix_sid2acn(ordering, sizeof(ordering)/sizeof(*ordering));
      routem =_matrix_router(NULL, ordering, sizeof(ordering)/sizeof(*ordering), 0);
    }
    break;
  default:break;
  }
  if(weightm&&routem&&reducem) {
    ambix_matrix_t*m0=ambix_matrix_multiply(weightm, routem, NULL);
    if(m0) {
      result=ambix_matrix_multiply(m0, reducem, NULL);
      ambix_matrix_destroy(m0);
    }
  }
  if(weightm) ambix_matrix_destroy(weightm);
  if(routem)  ambix_matrix_destroy(routem);
  if(reducem) ambix_matrix_destroy(reducem);

  if(0!=rows)
    return(result);

  if(result)
    ambix_matrix_destroy(result);
  return NULL;
}
