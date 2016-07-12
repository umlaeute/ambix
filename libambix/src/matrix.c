/* matrix.c -  Matrix handling              -*- c -*-

   Copyright © 2012-2016 IOhannes m zmölnig <zmoelnig@iem.at>.
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


ambix_matrix_t*
ambix_matrix_create(void) {
  return ambix_matrix_init(0, 0, NULL);
}
void
ambix_matrix_destroy(ambix_matrix_t*mtx) {
  ambix_matrix_deinit(mtx);
  free(mtx);
  mtx=NULL;
}
void
ambix_matrix_deinit(ambix_matrix_t*mtx) {
  uint32_t r;
  if(mtx->data) {
    for(r=0; r<mtx->rows; r++) {
      if(mtx->data[r])
        free(mtx->data[r]);
      mtx->data[r]=NULL;
    }
  }
  free(mtx->data);
  mtx->data=NULL;
  mtx->rows=0;
  mtx->cols=0;
}
ambix_matrix_t*
ambix_matrix_init(uint32_t rows, uint32_t cols, ambix_matrix_t*orgmtx) {
  ambix_matrix_t*mtx=orgmtx;
  uint32_t r;
  if(!mtx) {
    mtx=(ambix_matrix_t*)calloc(1, sizeof(ambix_matrix_t));
    if(!mtx)
      return NULL;
    mtx->data=NULL;
  }
  ambix_matrix_deinit(mtx);

  mtx->rows=rows;
  mtx->cols=cols;
  if(rows>0 && cols > 0) {
    mtx->data=(float32_t**)calloc(rows, sizeof(float32_t*));
    for(r=0; r<rows; r++) {
      mtx->data[r]=(float32_t*)calloc(cols, sizeof(float32_t));
    }
  }

  return mtx;
}

ambix_matrix_t*
_ambix_matrix_transpose(const ambix_matrix_t*matrix, ambix_matrix_t*xirtam) {
  uint32_t rows, cols, r, c;
  float32_t**mtx, **xtm;
  if(!xirtam)
    xirtam=ambix_matrix_init(matrix->cols, matrix->rows, NULL);

  rows=matrix->rows;
  cols=matrix->cols;

  mtx=matrix->data;
  xtm=xirtam->data;

  for(r=0; r<rows; r++)
    for(c=0; c<cols; c++)
      xtm[c][r]=mtx[r][c];

  return xirtam;
}

ambix_err_t
ambix_matrix_fill_data(ambix_matrix_t*mtx, const float32_t*ndata) {
  float32_t**matrix=mtx->data;
  uint32_t rows=mtx->rows;
  uint32_t cols=mtx->cols;
  uint32_t r;
  const float*data=(const float*)ndata;
  for(r=0; r<rows; r++) {
    uint32_t c;
    for(c=0; c<cols; c++) {
      matrix[r][c]=*data++;
    }
  }
  return AMBIX_ERR_SUCCESS;
}
ambix_err_t
_ambix_matrix_fill_data_byteswapped(ambix_matrix_t*mtx, const number32_t*data) {
  float32_t**matrix=mtx->data;
  uint32_t rows=mtx->rows;
  uint32_t cols=mtx->cols;
  uint32_t r;
  for(r=0; r<rows; r++) {
    uint32_t c;
    for(c=0; c<cols; c++) {
      number32_t v;
      number32_t d = *data++;
      v.i=swap4(d.i);
      matrix[r][c]=v.f;
    }
  }
  return AMBIX_ERR_SUCCESS;
}

#if 0
ambix_err_t
ambix_matrix_fill_data_transposed(ambix_matrix_t*mtx, const float32_t*data, int byteswap) {
  ambix_err_t err=AMBIX_ERR_SUCCESS;
  ambix_matrix_t*xtm=ambix_matrix_init(mtx->cols, mtx->rows, NULL);

  if(!xtm)
    return AMBIX_ERR_UNKNOWN;

  if(byteswap)
    err=_ambix_matrix_fill_data_byteswapped(xtm, (const number32_t*)data);
  else
    err=ambix_matrix_fill_data(xtm, data);

  if(AMBIX_ERR_SUCCESS==err) {
    ambix_matrix_t*resu=_ambix_matrix_transpose(mtx, xtm);
    if(!resu)
      err=AMBIX_ERR_UNKNOWN;
  }

  ambix_matrix_destroy(xtm);
  return err;
}
#endif



ambix_matrix_t*
ambix_matrix_copy(const ambix_matrix_t*src, ambix_matrix_t*dest) {
  if(!src)
    return NULL;
  if(!dest)
    dest=(ambix_matrix_t*)calloc(1, sizeof(ambix_matrix_t));

  if((dest->rows != src->rows) || (dest->cols != src->cols))
    ambix_matrix_init(src->rows, src->cols, dest);

  do {
    uint32_t r, c;
    float32_t**s=src->data;
    float32_t**d=dest->data;
    for(r=0; r<src->rows; r++) {
      for(c=0; c<src->cols; c++) {
        d[r][c]=s[r][c];
      }
    }
  } while(0);

  return dest;
}


ambix_matrix_t*
_ambix_matrix_multiply(const ambix_matrix_t*left, const ambix_matrix_t*right, ambix_matrix_t*dest) {
  uint32_t r, c, rows, cols, common;
  float32_t**ldat,**rdat,**ddat;
  float32_t lv, rv;
  if(!left || !right)
    return NULL;

  if(left->cols != right->rows)
    return NULL;

  if(!dest)
    dest=(ambix_matrix_t*)calloc(1, sizeof(ambix_matrix_t));

  if((dest->rows != left->rows) || (dest->cols != right->cols))
    ambix_matrix_init(left->rows, right->cols, dest);

  rows=dest->rows;
  cols=dest->cols;
  common=left->cols;

  ldat=left->data;
  rdat=right->data;
  ddat=dest->data;

  for(r=0; r<rows; r++)
    for(c=0; c<cols; c++) {
      double sum=0.;
      uint32_t i;
      for(i=0; i<common; i++) {
        lv=ldat[r][i];
        rv=rdat[i][c];
        sum+=lv*rv;
      }
      ddat[r][c]=(float32_t)sum;
    }

  return dest;
}

ambix_matrix_t*
ambix_matrix_fill(ambix_matrix_t*matrix, ambix_matrixtype_t typ) {
  int32_t rows=matrix->rows;
  int32_t cols=matrix->cols;
  int32_t r, c;
  float32_t**mtx=matrix->data;
  ambix_matrix_t*result=NULL;

  switch(typ) {
  default:
    return NULL;
  case (AMBIX_MATRIX_ZERO):
    for(r=0; r<rows; r++) {
      for(c=0; c<cols; c++)
        mtx[r][c]=0.;
    }
    break;
  case (AMBIX_MATRIX_ONE):
    for(r=0; r<rows; r++) {
      for(c=0; c<cols; c++)
        mtx[r][c]=1.;
    }
    break;
  case (AMBIX_MATRIX_IDENTITY):
    for(r=0; r<rows; r++) {
      for(c=0; c<cols; c++)
        mtx[r][c]=(float32_t)((r==c)?1.:0.);
    }
    break;

  case (AMBIX_MATRIX_FUMA): /* Furse Malham -> ACN/SN3D */
    result=_matrix_fuma2ambix(cols);
    if(!result)
      return NULL;
    matrix=ambix_matrix_copy(result, matrix);
    break;
  case (AMBIX_MATRIX_TO_FUMA): /* Furse Malham -> ACN/SN3D */
    result=_matrix_ambix2fuma(rows);
    if(!result)
      return NULL;
    matrix=ambix_matrix_copy(result, matrix);
    break;

  case (AMBIX_MATRIX_SID): /* SID -> ACN */ {
    float32_t*ordering=(float32_t*)malloc(rows*sizeof(float32_t));
    if(!ordering)
      return NULL;
    if(!_matrix_sid2acn(ordering, rows)) {
      free(ordering);
      return NULL;
    }
    matrix=_matrix_router(matrix, ordering, rows, 0);
    free(ordering);
  }
    break;
  case (AMBIX_MATRIX_TO_SID): /* ACN -> SID */ {
    float32_t*ordering=(float32_t*)malloc(rows*sizeof(float32_t));
    if(!_matrix_sid2acn(ordering, rows)) {
      free(ordering);
      return NULL;
    }
    matrix=_matrix_router(matrix, ordering, rows, 1);
    free(ordering);
  }
    break;


  case (AMBIX_MATRIX_N3D): /* N3D -> SN3D */ {
    float32_t*weights=NULL, *w_=NULL;
    int32_t counter=0;
    int32_t o=0, order=ambix_channels2order(rows);
    if(order<0)
      return NULL;
    weights=(float32_t*)malloc(rows*sizeof(float32_t));
    w_=weights;
    for(o=0; o<=order; o++) {
      const float32_t w=(float32_t)(1./sqrt(2.*o+1.));
      int32_t i;
      for(i=0; i<(2*o+1); i++) {
        *w_++=w;
        counter++;
      }
    }
    matrix=_matrix_diag(matrix, weights, rows);
    free(weights);
  }
    break;
  case (AMBIX_MATRIX_TO_N3D): /* SN3D -> N3D */ {
    float32_t*weights=NULL, *w_=NULL;
    int32_t counter=0;
    int32_t o, order=ambix_channels2order(rows);
    if(order<0)
      return NULL;
    weights=(float32_t*)malloc(rows*sizeof(float32_t));
    w_=weights;
    for(o=0; o<=order; o++) {
      const float32_t w=(float32_t)(sqrt(2.*o+1.));
      int32_t i;
      for(i=0; i<(2*o+1); i++) {
        *w_++=w;
        counter++;
      }
    }
    matrix=_matrix_diag(matrix, weights, rows);
    free(weights);
  }
    break;
  }
  if(result) ambix_matrix_destroy(result);

  return matrix;
}

ambix_matrix_t*
_ambix_matrix_pinv(const ambix_matrix_t*A, ambix_matrix_t*P) {
  const float32_t eps=1e-7;
  ambix_matrix_t *result = NULL;

  /* try cholesky inversion */
  result=_ambix_matrix_pinvert_cholesky(A, P, eps);
  if(result)
    return result;

  /* if that fails (should never happen), fall back to gauss-jordan */
  if (A->rows==A->cols) {
    ambix_matrix_t*Ax = ambix_matrix_copy(A, NULL);
    result = _ambix_matrix_invert_gaussjordan(Ax, P, eps); // do normal inverse if square matrix
    if(Ax)   ambix_matrix_destroy(Ax);
  } else {
    /* we'll have to do the pseudo-inverse:
     * P=A'*inv(A*A') if row<col
     * P=inv(A'*A)*A' if col<row
     */
    ambix_matrix_t *At = NULL;
    ambix_matrix_t *temp = NULL;
    ambix_matrix_t *temp2 = NULL;
    do {
      At = _ambix_matrix_transpose(A, At);

      if (A->rows > A->cols) {
        temp = _ambix_matrix_multiply(At, A, NULL);
        if (!temp)break;
        temp2 = _ambix_matrix_invert_gaussjordan(temp, temp2, eps);
        if (!temp2)break;

        result = _ambix_matrix_multiply(temp2, At, P);
      } else {
        temp = _ambix_matrix_multiply(A, At, NULL);
        if (!temp)break;
        temp2 = _ambix_matrix_invert_gaussjordan(temp, temp2, eps);
        if (!temp2)break;

        result = _ambix_matrix_multiply(At, temp2, P);
      }
    } while (0);
    if(At)   ambix_matrix_destroy(At);
    if(temp) ambix_matrix_destroy(temp);
    if(temp2)ambix_matrix_destroy(temp2);
  }

  return result;
}

#define MTXMULTIPLY_DATA_FLOAT(typ)                                     \
  ambix_err_t ambix_matrix_multiply_##typ(typ##_t*dest, const ambix_matrix_t*matrix, const typ##_t*source, int64_t frames) { \
    float32_t**mtx=matrix->data;                                        \
    const uint32_t outchannels=matrix->rows;                            \
    const uint32_t inchannels=matrix->cols;                             \
    int64_t frame;                                                      \
    typ##_t*dst=dest;                                                   \
    const typ##_t*src=source;                                           \
    for(frame=0; frame<frames; frame++) {                               \
      uint32_t outchan;                                                 \
      for(outchan=0; outchan<outchannels; outchan++) {                  \
        double sum=0.;                                                  \
        uint32_t inchan;                                                \
        for(inchan=0; inchan<inchannels; inchan++) {                    \
          double scale=mtx[outchan][inchan];                            \
          double in=src[frame*inchannels+inchan];                       \
          sum+=scale*in;                                                \
        }                                                               \
        dst[frame*outchannels+outchan]=(typ##_t)sum;                    \
      }                                                                 \
    }                                                                   \
    return AMBIX_ERR_SUCCESS;                                           \
  }                                                                     \

MTXMULTIPLY_DATA_FLOAT(float32);
MTXMULTIPLY_DATA_FLOAT(float64);

#define MTXMULTIPLY_DATA_INT(typ)                                       \
  ambix_err_t ambix_matrix_multiply_##typ(typ##_t*dest, const ambix_matrix_t*matrix, const typ##_t*source, int64_t frames) { \
    float32_t**mtx=matrix->data; \
    const uint32_t outchannels=matrix->rows;                            \
    const uint32_t inchannels=matrix->cols;                             \
    int64_t frame;                                                      \
    for(frame=0; frame<frames; frame++) {                               \
      uint32_t outchan;                                                 \
      typ##_t*dst=dest+frame;                                           \
      const typ##_t*src=source+frame;                                   \
      for(outchan=0; outchan<outchannels; outchan++) {                  \
        double sum=0.;                                                  \
        uint32_t inchan;                                                \
        for(inchan=0; inchan<inchannels; inchan++) {                    \
          double scale=mtx[outchan][inchan];                            \
          double in=src[inchan*frames];                                 \
          sum+=scale * in;                                              \
        }                                                               \
        dst[frames*outchan]=(typ##_t)(sum);  /* FIXXXME: saturation */  \
      }                                                                 \
    }                                                                   \
    return AMBIX_ERR_SUCCESS;                                           \
  }

MTXMULTIPLY_DATA_INT(int16);
MTXMULTIPLY_DATA_INT(int32);



/* conversion matrices
 * conversion=(weights*ordering)
 */

/* weights
 *  N3D = SN3D * sqrt(2n+1)
 * SN3D =  N3D / sqrt(2n+1)
 */

/* ordering
 */


ambix_matrix_t*_matrix_diag(ambix_matrix_t*orgmatrix, const float32_t*diag, uint32_t count) {
  uint32_t i;
  ambix_matrix_t*matrix=ambix_matrix_init(count, count, orgmatrix);
  for(i=0; i<count; i++)
    matrix->data[i][i]=diag[i];

  return matrix;
}
ambix_matrix_t*_matrix_router(ambix_matrix_t*orgmatrix, const float32_t*route, uint32_t count, int swap) {
  uint32_t i;
  ambix_matrix_t*matrix=NULL;
  for(i=0; i<count; i++) {
    uint32_t o=(uint32_t)route[i];
    if(route[i]<0. || o>count)
      return NULL;
  }
  matrix=ambix_matrix_init(count, count, orgmatrix);
  for(i=0; i<count; i++) {
    uint32_t o=(uint32_t)route[i];
    if(swap)
      matrix->data[o][i]=1.;
    else
      matrix->data[i][o]=1.;
  }
  return matrix;
}


ambix_matrix_t*_matrix_permutate(ambix_matrix_t*matrix, const float32_t*route, int swap) {
  uint32_t i;
  uint32_t count   =swap?matrix->cols:matrix->rows;
  uint32_t maxroute=swap?matrix->rows:matrix->cols;

  for(i=0; i<count; i++) {
    uint32_t o=(uint32_t)route[i];
    if(o>maxroute)
      return NULL;
  }
  for(i=0; i<count; i++) {
    if(route[i]>=-0.1) {
      uint32_t o=(uint32_t)route[i];
      if(swap)
        matrix->data[o][i]=1.;
      else
        matrix->data[i][o]=1.;
    }
  }
  return matrix;
}


ambix_matrix_t*
ambix_matrix_pinv(const ambix_matrix_t*A, ambix_matrix_t*P) {
  return _ambix_matrix_pinv(A, P);
}
ambix_matrix_t*
ambix_matrix_multiply(const ambix_matrix_t*left, const ambix_matrix_t*right, ambix_matrix_t*dest) {
  return _ambix_matrix_multiply(left, right, dest);
}
