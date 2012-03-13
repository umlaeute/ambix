#ifndef TESTS_COMMON_H
#define TESTS_COMMON_H

#include <ambix/ambix.h>

#include <stdlib.h>
static inline void pass(void) {exit(0); }
static inline void fail(void) {exit(1); }
static inline void skip(void) {exit(77); }

#include <stdio.h>
#include <stdarg.h>
static inline void pass_if (int test, int line, const char *format, ...)
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
} /* pass_if */
static inline void skip_if (int test, int line, const char *format, ...)
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
} /* skip_if */
static inline void fail_if (int test, int line, const char *format, ...)
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
} /* fail_if */

void matrix_print(const ambixmatrix_t*mtx);
float32_t matrix_diff(uint32_t line, const ambixmatrix_t*A, const ambixmatrix_t*B, float32_t eps);

void data_print(const float32_t*data, uint64_t frames);
float32_t data_diff(uint32_t line, const float32_t*A, const float32_t*B, uint64_t frames, float32_t eps);

float32_t*data_sine(uint64_t frames, uint32_t channels, float32_t periods);

#define STARTTEST()   printf("============ %s[%04d]:\t%s ==========\n", __FILE__, __LINE__, __FUNCTION__)

#endif /* TESTS_COMMON_H */

