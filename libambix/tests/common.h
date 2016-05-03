/* tests/common.h -  AMBIsonics eXchange Library test utilities              -*- c -*-

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

#ifndef TESTS_COMMON_H
#define TESTS_COMMON_H

#include <ambix/ambix.h>

#include <stdlib.h>
static inline void pass(void) {exit(0); }
static inline void fail(void) {exit(1); }
static inline void skip(void) {exit(77); }

#include <stdio.h>
#include <stdarg.h>
#define MARK() printf("%s:%d[%s]\n", __FILE__, __LINE__, __FUNCTION__)

static inline int pass_if (int test, int line, const char *format, ...)
{
  if (test) {
    va_list argptr ;
    printf("@%d: ", line);
    va_start (argptr, format) ;
    vprintf (format, argptr) ;
    va_end (argptr) ;
    printf("\n");
    pass();
  } ;
  return test;
} /* pass_if */
static inline int skip_if (int test, int line, const char *format, ...)
{
  if (test) {
    va_list argptr ;
    printf("@%d: ", line);
    va_start (argptr, format) ;
    vprintf (format, argptr) ;
    va_end (argptr) ;
    printf("\n");
    skip();
  } ;
  return test;
} /* skip_if */
static inline int fail_if (int test, int line, const char *format, ...)
{
  if (test) {
    va_list argptr ;
    printf("@%d: ", line);
    va_start (argptr, format) ;
    vprintf (format, argptr) ;
    va_end (argptr) ;
    printf("\n");
    fail();
  } ;
  return test;
} /* fail_if */

void matrix_print(const ambix_matrix_t*mtx);
float32_t matrix_diff(uint32_t line, const ambix_matrix_t*A, const ambix_matrix_t*B, float32_t eps);

void data_print(const float32_t*data, uint64_t frames);
float32_t data_diff(uint32_t line, const float32_t*A, const float32_t*B, uint64_t frames, float32_t eps);

void data_transpose(float32_t*outdata, const float32_t*indata, uint32_t inrows, uint32_t incols);
float32_t*data_sine(uint64_t frames, uint32_t channels, float32_t periods);
float32_t*data_ramp(uint64_t frames, uint32_t channels);

#define STRINGIFY(x) #x
#define STARTTEST(x)   printf("<<< running TEST %s[%04d]:\t%s '%s'\n", __FILE__, __LINE__, __FUNCTION__, x)
#define STOPTEST(x)    printf(">>> test SUCCESS %s[%04d]:\t%s '%s'\n", __FILE__, __LINE__, __FUNCTION__, x)


#endif /* TESTS_COMMON_H */

