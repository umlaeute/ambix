/* data - test generic data functionality

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
#include <string.h>
#include <stdlib.h>

static float32_t left[]= {
   0.19, 0.06, 0.14,
   0.05, 0.08, 0.44,
   0.25, 0.90, 0.77,
   0.83, 0.51, 0.58,
};

void do_diff(float32_t eps) {
  float32_t errf;
  unsigned int i;
  unsigned int size=sizeof(left)/sizeof(*left);
  float32_t*right=malloc(sizeof(left));
  float32_t maxeps=eps;

  STARTTEST("\n");
  data_print(FLOAT32, left, size);

  /* comparisons:
       - left/right data is NULL
     - non-failing tests:
       - all values diff==0
       - all values diff<eps
       - few values diff<eps
       - many values diff<eps
  */
  /* compare equal data */
  STARTTEST("ident\n");
  errf=data_diff(__LINE__, FLOAT32, left, left, size, eps);
  fail_if(errf>0.f, __LINE__, "diffing mtx with itself returned %g (>%g)", errf, 0.f);

  /* compare equal data */
  STARTTEST("equal\n");
  for(i=0; i<size; i++) {
    right[i]=left[i];
  }
  errf=data_diff(__LINE__, FLOAT32, left, right, size, eps);
  fail_if(errf>0.f, __LINE__, "diffing mtx with copy returned %g (>%g)", errf, 0.f);

  /* compare data where all values differ, but <eps */
  STARTTEST("all<eps\n");
  for(i=0; i<size; i++) {
    right[i]=left[i]+eps*0.5;
  }
  errf=data_diff(__LINE__, FLOAT32, left, right, size, eps);
  fail_if(errf>eps, __LINE__, "diffing mtx with mtx+eps/2 returned %g (>%g)", errf, size, eps);
  for(i=0; i<size; i++) {
    right[i]=left[i]-eps*0.5;
  }
  errf=data_diff(__LINE__, FLOAT32, left, right, size, eps);
  fail_if(errf>eps, __LINE__, "diffing mtx with mtx-eps/2 returned %g (>%g)", errf, size, eps);

  /* compare data where many values differ with <eps; but one with >eps */
  STARTTEST("most<eps;one>eps\n");
  for(i=0; i<size; i++) {
    right[i]=left[i];
  }
  for(i=0; i<(size/2); i++) {
    right[i]=left[i]+eps*0.5;
  }
  right[0]=left[0]+eps*1.5;
  errf=data_diff(__LINE__, FLOAT32, left, right, size, eps);
  fail_if(errf>(eps*2.0), __LINE__, "diffing mtx with one value>eps returned %g (>%g)", errf, size, eps);
  fail_if(errf<(eps*1.0), __LINE__, "diffing mtx with one value>eps returned %g (>%g)", errf, size, eps);

  /* compare data where most values differ with >eps */
  STARTTEST("most>eps\n");
  for(i=0; i<size; i++) {
    right[i]=left[i];
  }
  maxeps=eps*1.5;
  for(i=0; i<(size-1); i++) {
    right[i]=left[i]-maxeps;
  }
  errf=data_diff(__LINE__, FLOAT32, left, right, size, eps);
  fail_if(errf<eps*1.0, __LINE__, "diffing mtx with one value>eps returned %g (<%g)", errf, eps*1.0);
  fail_if(errf>eps*2.0, __LINE__, "diffing mtx with one value>eps returned %g (<%g)", errf, eps*2.0);

  free(right);
  STOPTEST("\n");
}

int main(int argc, char**argv) {
  do_diff(1e-1);
  do_diff(1e-7);

  return pass();
}
