/* matrix_invert.c -  utilities for matrix inversion              -*- c -*-

   Copyright © 2016 IOhannes m zmölnig <zmoelnig@iem.at>.
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

#include "private.h"

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include <math.h>

/**
 * simple matrix inversion of square matrices, using Gauss-Jordan
 */
/* this modifies the input matrix! */
ambix_matrix_t*
_ambix_matrix_invert_gaussjordan(ambix_matrix_t*input, ambix_matrix_t*inverse, float32_t eps)
{
  ambix_matrix_t*inverse_org = inverse;
  int i, k;
  float32_t *a1, *b1, *a2, *b2;

  int errors=0; /* error counter */

  if(input==0)
    { // no input matrix
      return NULL;
    }

  if (input->cols != input->rows)
    {// matrix is not squared
      return NULL;
    }

  int col=input->cols, row=input->rows;

  /* 1a reserve space for the inverted matrix */
  if(!inverse)
    {
      inverse=ambix_matrix_init(row, col, NULL);
    }

  float32_t **original=input->data;
  float32_t **inverted=inverse->data;


  /* 1b make an eye-shaped float-buf for B */
  ambix_matrix_fill(inverse, AMBIX_MATRIX_IDENTITY);

  /* 2. do the Gauss-Jordan */
  //printf("GaussJordan\n");
  for (k=0; k<row; k++) {
    /* adjust current row */
    float32_t diagel = original[k][k];
    float32_t i_diagel = 0;
    if(diagel>-eps && diagel<eps)
      errors++;
    else
      i_diagel = 1./diagel;

    /* normalize current row (set the diagonal-element to 1 */
    for (i=0; i < row; i++)
      {
        original[k][i] *= i_diagel;
        inverted[k][i] *= i_diagel;
      }

    /* eliminate the k-th element in each row by adding the weighted normalized row */
    for (i=0; i < row; i++)
      {
        if (i-k)
          {
            float32_t f =-original[i][k];
            int j;
            for (j=row-1; j >= 0; j--) {
              original[i][j] += f * original[k][j];
              inverted[i][j] += f * inverted[k][j];
            }
          }
      }
  }

  if (errors > 0) {
    if(inverse != inverse_org)
      /* if the 'inverse' was locally allocated, free it */
      ambix_matrix_destroy(inverse);
    inverse=NULL;
  }

  return inverse;
}

/**
 * matrix inversion using the Cholesky algorithm
 *
 * the functions _am_cholesky2_decomp and _am_cholesky_2_inverse
 * have been extracted from the "Survival" package for R.
 *   https://cran.r-project.org/web/packages/survival/index.html
 *
 * They are distributed under the Lesser Gnu General Public License 2 (or greater)
 * and are
 * © 2009 Thomas Lumley
 * © 2009-2016 Terry M Therneau
 */
/** cholesky2 decomposition
 * \author © -2009 Thomas Lumley
 * \author © 2009-2016 Terry M Therneau <therneau.terry@mayo.edu>
 * \copyright LGPL >= 2
 * \note origin survival 2.39-3 https://cran.r-project.org/web/packages/survival/index.html
 */
static int _am_cholesky2_decomp(ambix_matrix_t*mtx, float32_t toler)
{
  const uint32_t columns = mtx->cols;
  float32_t**matrix= mtx->data;
  float32_t temp;
  int i, j, k;
  float32_t eps, pivot;
  int rank;
  int nonneg;

  nonneg = 1;
  eps = 0;
  for (i = 0; i < columns; i++) {
    if (matrix[i][i] > eps)
      eps = matrix[i][i];
    for (j = (i + 1); j < columns; j++)
      matrix[j][i] = matrix[i][j];
  }
  eps *= toler;

  rank = 0;
  for (i = 0; i < columns; i++) {
    pivot = matrix[i][i];
    if (pivot < eps) {
      matrix[i][i] = 0;
      if (pivot < -8 * eps)
        nonneg = -1;
    } else {
      rank++;
      for (j = (i + 1); j < columns; j++) {
        temp = matrix[j][i] / pivot;
        matrix[j][i] = temp;
        matrix[j][j] -= temp * temp * pivot;
        for (k = (j + 1); k < columns; k++)
          matrix[k][j] -= temp * matrix[k][i];
      }
    }
  }
  return (rank * nonneg);
}
/* inplace inverse */
/** inplace inversion of a cholesky2 decomposed matrix
 * \author © -2009 Thomas Lumley
 * \author © 2009-2016 Terry M Therneau <therneau.terry@mayo.edu>
 * \copyright LGPL >= 2
 * \note origin survival 2.39-3 https://cran.r-project.org/web/packages/survival/index.html
 */
static void _am_cholesky2_inverse(ambix_matrix_t*mtx)
{
  const int columns = mtx->cols;;
  float32_t**matrix = mtx->data;
  register float32_t temp;
  register int i, j, k;

  /*
  ** invert the cholesky in the lower triangle
  **   take full advantage of the cholesky's diagonal of 1's
  */
  for (i = 0; i < columns; i++) {
    if (matrix[i][i] > 0) {
      matrix[i][i] = 1 / matrix[i][i]; /*this line inverts D */
      for (j = (i + 1); j < columns; j++) {
        matrix[j][i] = -matrix[j][i];
        for (k = 0; k < i; k++) /* sweep operator */
          matrix[j][k] += matrix[j][i] * matrix[i][k];
      }
    }
  }

  /*
  ** lower triangle now contains inverse of cholesky
  ** calculate F'DF (inverse of cholesky decomp process) to get inverse
  **   of original matrix
  */
  for (i = 0; i < columns; i++) {
    if (matrix[i][i] == 0) { /* singular row */
      for (j = 0; j < i; j++)
        matrix[j][i] = 0;
      for (j = i; j < columns; j++)
        matrix[i][j] = 0;
    } else {
      for (j = (i + 1); j < columns; j++) {
        temp = matrix[j][i] * matrix[j][j];
        if (j != i)
          matrix[i][j] = temp;
        for (k = i; k < j; k++)
          matrix[i][k] += temp * matrix[j][k];
      }
    }
  }

  // ugly fix to return only inverse
  for (i = 1; i < columns; i++)
    for (j = 0; j < i; j++)
      matrix[i][j] = matrix[j][i];
}

/*
 * calculate the inverse of any (rectangular) real-valued matrix using cholesky decomposition
 */
ambix_matrix_t*
_ambix_matrix_pinvert_cholesky(const ambix_matrix_t*input, ambix_matrix_t*inverse, float32_t tolerance) {
  /* (rows>cols)?(inv(x'*x)*x'):(x'*inv(x*x')) */
  float32_t toler = tolerance;
  ambix_matrix_t*trans = _ambix_matrix_transpose(input, 0);
  ambix_matrix_t*chinv=0;
  ambix_matrix_t*result=0;
  do {
    if(!trans)break;
    if(input->rows > input->cols) {
      chinv=_ambix_matrix_multiply(trans, input, chinv);
      if(!chinv)break;
      _am_cholesky2_decomp(chinv, toler);
      _am_cholesky2_inverse(chinv);
      result=_ambix_matrix_multiply(chinv, trans, inverse);
    } else {
      chinv=_ambix_matrix_multiply(input, trans, chinv);
      if(!chinv)break;
      _am_cholesky2_decomp(chinv, toler);
      _am_cholesky2_inverse(chinv);
      result=_ambix_matrix_multiply(trans, chinv, inverse);
    }
  } while(0);
  if(trans)ambix_matrix_destroy(trans);
  if(chinv)ambix_matrix_destroy(chinv);

  return result;
}
