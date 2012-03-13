#ifndef TESTS_COMMON_H
#define TESTS_COMMON_H

#include <ambix/ambix.h>

#include <stdlib.h>
static void pass(void) {exit(0); }
static void fail(void) {exit(1); }
static void skip(void) {exit(77); }

#endif /* TESTS_COMMON_H */

