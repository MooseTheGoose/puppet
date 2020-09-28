#include "puppet_types.hpp"
#include "webdrivers.hpp"
#include <stdlib.h>

int main(int argc, const char *argv[]) {
  GeckoPuppet puppet;

  if(!puppet.init()) {
    printf("I got the puppet connected\r\n");
    printf("Puppet Local Address: %s\r\n", puppet.local_addr);
    printf("Quitting...\r\n");
    if(!puppet.quit()) {
      printf("Succeeded in quitting!\r\n");
    } else {
      printf("Failed in quitting...\r\n");
    }
  } else {
	printf("OHNO");
  }
  return 0;
}
