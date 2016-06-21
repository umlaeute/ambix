/* common test functionality

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

#include "common.h"
#include <math.h>

void matrix_print(const ambix_matrix_t*mtx) {
  printf("matrix[%p] ", mtx);
  printf(" [%dx%d]@%p\n", mtx->rows, mtx->cols, mtx->data);
  if(mtx->data) {
    uint32_t c, r;
    for(r=0; r<mtx->rows; r++) {
      for(c=0; c<mtx->cols; c++)
        printf(" %05f", mtx->data[r][c]);
      printf("\n");
    }
  }
}
#define MAX_OVER 10
float32_t matrix_diff(uint32_t line, const ambix_matrix_t*A, const ambix_matrix_t*B, float32_t eps) {
  uint32_t r, c;
  float32_t sum=0.;
  float32_t maxdiff=-1.f;
  uint64_t overcount=0;

  float32_t**a=NULL;
  float32_t**b=NULL;

  fail_if((NULL==A), line, "left-hand matrix of matrixdiff is NULL");
  fail_if((NULL==B), line, "right-hand matrix of matrixdiff is NULL");
  fail_if(((A->rows!=B->rows) || (A->cols!=B->cols)), line, "matrixdiff matrices do not match [%dx%d]!=[%dx%d]", A->rows, A->cols, B->rows, B->cols);

  a=A->data;
  b=B->data;
  for(r=0; r<A->rows; r++)
    for(c=0; c<B->cols; c++) {
      float32_t v=a[r][c]-b[r][c];
      float32_t vabs=(v<0)?-v:v;
      fail_if(isnan(v), line, "[%d|%d] is NaN: %f <-> %f", r, c, a[r][c], b[r][c]);
      if(vabs>maxdiff)
        maxdiff=vabs;
      sum+=vabs;
      if(vabs>eps) {
        overcount++;
        if(overcount<MAX_OVER)
          printf("%+f - %+f = %+g @ [%02d|%02d]\n", a[r][c], b[r][c], v, r, c);
      }
    }

  if(overcount>MAX_OVER)
    printf("[%d] accumulated error %f over %d/%d frames (eps=%g)\n", line, sum, (int)overcount, (int)(A->cols*B->rows), eps);
  return maxdiff;
}


float32_t data_diff(uint32_t line, const float32_t*A, const float32_t*B, uint64_t frames, float32_t eps) {
  uint64_t i;
  float32_t sum=0.;
  float32_t maxdiff=-1.f;
  uint64_t overcount=0;

  fail_if((NULL==A), line, "left-hand data of datadiff is NULL");
  fail_if((NULL==B), line, "right-hand data of datadiff is NULL");

  for(i=0; i<frames; i++) {
    float32_t v=A[i]-B[i];
    float32_t vabs=(v<0)?-v:v;
    fail_if(isnan(v), line, "[%d] is NaN: %f <-> %f", i, A[i], B[i]);
    if(vabs>maxdiff)
      maxdiff=vabs;
    sum+=vabs;
    if(vabs>eps) {
      overcount++;
      if(overcount<MAX_OVER)
        printf("%+f - %+f=%g @ %d\n", A[i], B[i], v, (int)i);
    }

  }

  if(overcount>MAX_OVER)
    printf("[%d] accumulated error %f over %d/%d frames (eps=%g)\n", line, sum, (int)overcount, (int)frames, eps);

  return maxdiff;
}


void data_print(const float32_t*data, uint64_t frames) {
  uint64_t i;
  for(i=0; i<frames; i++) {
    float f=*data++;
    printf("%05d: %f\n", (int)i, f);
  }
}

void data_transpose(float32_t*outdata, const float32_t*indata, uint32_t inrows, uint32_t incols) {
  uint32_t r, c;
  for(r=0; r<inrows; r++) {
    for(c=0; c<incols; c++) {
      uint32_t outoffset=inrows*c+r;
      uint32_t inoffset=incols*r+c;
      //      printf("writing %d [%f] to %d\n", inoffset, indata[inoffset], outoffset);
      outdata[outoffset]=indata[inoffset];
    }
  }
}

float32_t float32cast(ambixtest_presentationformat_t fmt, float32_t value) {
   switch(fmt) {
     case INT16: return (float32_t)((int16_t)(value*32768.))/32768.;
     case INT32: return (float32_t)((int32_t)(value*2147483648.))/2147483648.;
     default   : break;
   }
   return value;
}


float32_t*data_sine(ambixtest_presentationformat_t fmt, uint64_t frames, uint32_t channels, float32_t freq) {
  float32_t periods=freq/44100.;
  float32_t*data=(float32_t*)calloc(frames*channels, sizeof(float32_t));
  float32_t*datap=data;
  int64_t frame;
  for(frame=0; frame<frames; frame++) {
    int32_t chan;
    float32_t f=(float32_t)frame*periods;
    float32_t value=float32cast(fmt, 0.5*sinf(f));
    for(chan=0; chan<channels; chan++)
      *datap++=value;
  }
  return data;
}

float32_t*data_ramp(ambixtest_presentationformat_t fmt, uint64_t frames, uint32_t channels) {
  float32_t*data=(float32_t*)calloc(frames*channels, sizeof(float32_t));
  float32_t*datap=data;
  double increment=1./(double)frames;
  double value=0.;
  int64_t frame;
  for(frame=0; frame<frames; frame++) {
    int32_t chan;
    float32_t v32=float32cast(fmt, value-0.5);
    value+=increment;
    for(chan=0; chan<channels; chan++)
      *datap++=v32;
  }
  return data;
}



