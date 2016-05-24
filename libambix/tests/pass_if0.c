#include "common.h"

int main(int argc, char**argv) {

  pass_if(0, __LINE__, "should %s pass", "NOT");
  return 0;
}
