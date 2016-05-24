#include "common.h"

int main(int argc, char**argv) {

  fail_if(0, __LINE__, "should %s fail", "NOT");
  return 0;
}
