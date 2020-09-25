#include <string>
#include "puppet_types.hpp"

int main(int argc, const char *argv[]) {
  printf("%d\r\n", is_prefix(argv[1], argv[2]));
  return 0;
}
