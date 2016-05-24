#include "common.h"

int main(int argc, char**argv) {

  skip_if(0, __LINE__, "should %s skip", "NOT");
  return 0;
}
