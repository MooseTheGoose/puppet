#include "webdrivers.hpp"
#include "puppet_types.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(_WIN32)
#include <windows.h>
#define PUPPET_SLEEP(x) Sleep(x)
#else
#include <unistd.h>
#define PUPPET_SLEEP(x) usleep((x) * 1000)
#endif

/*
 *  An OS-specific thing which uses some OS utility 
 *  to get the first local address that a process is using. 
 */
static const char *local_addr_from_process(const PuppetProcess &process) {
  const char *line_sep;
  int len_sep;
  char *local_address = 0;
  PuppetPipedProcess netstat;


  #if defined(_WIN32)
  int pid = process.pid;
  netstat.init("netstat -ano -p tcp");
  line_sep = "\n";
  #else
  int pid = process.pid;
  netstat.init("lsof -nP -iTCP");
  line_sep = "\n";
  #endif

  if(!netstat.identifiable()) {
	return 0;
  }

  len_sep = strlen(line_sep);
  const char *data = netstat.output;
  vector< vector<string> > lines = vector< vector<string> >();
  netstat.wait();

  while(*data) {
    vector<string> line_fields = vector<string>();

    while(!is_prefix(line_sep, data) && *data) {
      while(isspace(*data) && !is_prefix(line_sep, data)) {
        data++;
      }
      if(*data && !is_prefix(line_sep, data)) {
        string field = "";
        while(!isspace(*data) && *data && !is_prefix(line_sep, data)) {
          field.push_back(*data++);
        }
        line_fields.push_back(field);
      }
    }
    if(is_prefix(line_sep, data)) {
      data += len_sep;
    }
    lines.push_back(line_fields);  
  }

  #if defined(_WIN32)
  for(size_t i = 0; i < lines.size(); i++) {
    vector<string> fields = lines[i];
    if(fields.size() == 5 && !fields[0].compare("TCP")) {
      if(std::stoi(fields[4]) == pid) {
        local_address = new char[fields[1].size() + 1];
	fields[1].copy(local_address, fields[1].size(), 0);
	local_address[fields[1].size()] = 0;

        goto finish;
      }
    }
  }
  #else
  for(size_t i = 0; i < lines.size(); i++) {
    vector<string> fields = lines[i];
    if(fields.size() == 10 && !fields[7].compare("TCP")) {
      if(std::stoi(fields[1]) == pid) {
        size_t addrsize = 0;
        while(fields[8][addrsize++] != ':') {}
        while(isdigit(fields[8][addrsize++])) {}

        local_address = new char[addrsize--];
        fields[8].copy(local_address, addrsize, 0);
        local_address[addrsize] = 0;

        goto finish;
      }
    }
  }
  #endif

  finish:
    netstat.free_output();
    return local_address;
}


int GeckoPuppet::init() {
  this->session = 0;
  this->local_addr = 0;
  PuppetProcess gecko_driver;
  int status = -1;

  gecko_driver.init("geckodriver --port=0");
  if(gecko_driver.identifiable()) {
    /*
     *  Fun fact: 
     *  You have to wait for the server to spin up...
     *  Sleep for 100 ms to give them the time they need.
     */
    PUPPET_SLEEP(100);

    const char *gecko_addr = local_addr_from_process(gecko_driver);
    if(gecko_addr) {
      this->driver = gecko_driver;
      this->local_addr = gecko_addr;
      if((this->session = curl_easy_init())) {
        status = 0;
      } else {
        delete[] this->local_addr;
        this->local_addr = 0;
      }
    }
  }

  return status;
}

int GeckoPuppet::quit() {
  if(this->session) {
    curl_easy_cleanup(this->session);
    this->session = 0;
  }
  if(this->local_addr) {
    delete[] this->local_addr;
    this->local_addr = 0;
  }
  return this->driver.murder();
}
