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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>
#include <stdlib.h>
static inline int my_exit(int i) {exit(i); return i;}
static inline int pass(void) {return my_exit(0); }
static inline int fail(void) {return my_exit(1); }
static inline int skip(void) {return my_exit(77); }

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
    return pass();
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
    return skip();
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
    return fail();
  } ;
  return test;
} /* fail_if */
static inline int print_if (int test, int line, const char *format, ...)
{
  if (test) {
    va_list argptr ;
    printf("@%d: ", line);
    va_start (argptr, format) ;
    vprintf (format, argptr) ;
    va_end (argptr) ;
    printf("\n");
  }
  return test;
} /* print_if */

typedef enum {
 INT16,
 INT32,
 FLOAT32,
 FLOAT64
} ambixtest_presentationformat_t;


void matrix_print(const ambix_matrix_t*mtx);
float32_t matrix_diff(uint32_t line, const ambix_matrix_t*A, const ambix_matrix_t*B, float32_t eps);

void data_print(ambixtest_presentationformat_t fmt, const void*data, uint64_t frames);
float32_t data_diff(uint32_t line, ambixtest_presentationformat_t fmt, const void*A, const void*B, uint64_t frames, float32_t eps);

size_t data_size(ambixtest_presentationformat_t fmt);
void*data_calloc(ambixtest_presentationformat_t fmt, size_t nmembers);
void data_transpose(float32_t*outdata, const float32_t*indata, uint32_t inrows, uint32_t incols);
void*data_sine(ambixtest_presentationformat_t fmt, uint64_t frames, uint32_t channels, float32_t periods);
void*data_ramp(ambixtest_presentationformat_t fmt, uint64_t frames, uint32_t channels);

int64_t ambixtest_readf (ambix_t *ambix, ambixtest_presentationformat_t fmt,
                         void*ambidata , uint64_t ambioffset,
                         void*otherdata, uint64_t otheroffset,
                         int64_t frames);
int64_t ambixtest_writef (ambix_t *ambix, ambixtest_presentationformat_t fmt,
                          const void*ambidata , const uint64_t ambioffset,
                          const void*otherdata, const uint64_t otheroffset,
                          int64_t frames);
int ambixtest_rmfile(const char*path);
int ambixtest_uniquenumber(void);
/* write uniquish filename into 'inbuf' and return a pointer to it
 * if ext is NULL, it defaults to '.caf'*/
char*ambixtest_getfname(char*inbuf, size_t length, const char*path, const char*basename, const char*ext);
#define FILENAME_MAIN ambixtest_getfname(alloca(1024), 1024, 0, argv[0], 0)
#define FILENAME_FILE ambixtest_getfname(alloca(1024), 1024, 0, __FILE__, 0)

#define STRINGIFY(x) #x
#define STARTTEST   printf("<<< running TEST %s[%04d]:%s\t", __FILE__, __LINE__, __FUNCTION__),printf
#define STOPTEST    printf(">>> test SUCCESS %s[%04d]:%s\t", __FILE__, __LINE__, __FUNCTION__),printf


#endif /* TESTS_COMMON_H */

