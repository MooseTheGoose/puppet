#include "puppet_types.hpp"
#include <windows.h>

int main(int argc, const char *argv[]) {
  PuppetProcess pp = PuppetProcess("geckodriver --port 0");
  Sleep(1000);
  PuppetPipedProcess ppp = PuppetPipedProcess("netstat -ano -p tcp");

  if(pp.pid > 0 && ppp.pid > 0) {
    printf("Huzzah, I did it! GeckoDriver id = %d\r\n", pp.pid);
    ppp.output.push_back(0);
    printf("%s\r\n", ppp.output.data());
  } else {
    printf("Oh no. I didn't do the thing: %d\r\n", GetLastError());
  }

  return 0;
}
