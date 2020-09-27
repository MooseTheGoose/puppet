#include "puppet_types.hpp"

int main(int argc, const char *argv[]) {

  PuppetProcess pp = PuppetProcess("geckodriver --port 0");
  PuppetPipedProcess ppp = PuppetPipedProcess("lsof -i4TCP");

  if(pp.pid > 0 && ppp.pid > 0) {
    printf("Huzzah, I did it! GeckoDriver id = %d\r\n", pp.pid);
    ppp.output.push_back(0);
    printf("%s\r\n", ppp.output.data());
  } else {
    printf("Oh no. I didn't do the thing\r\n");
  }

  return 0;
}
