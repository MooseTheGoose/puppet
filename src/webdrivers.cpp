#include "webdrivers.hpp"
#include "puppet_types.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*
 *  An OS-specific thing which uses some OS utility 
 *  to get the port from the process ID. 
 *
 *  TODO: Have Linux and Mac OS use and parse lsof.
 */
static const char *local_addr_from_pid(int pid) {
  const char *line_sep;
  int len_sep;
  char *local_address = 0;
  PuppetPipedProcess netstat;

  #if defined(_WIN32)
  netstat.init("netstat -ano -p tcp");
  line_sep = "\n";
  #else
  netstat.init("lsof -nP -iTCP");
  line_sep = "\n";
  #endif

  if(netstat.process.pid <= 0) {
	return 0;
  }

  len_sep = strlen(line_sep);
  const char *data = netstat.output;
  vector<vector<string>> lines = vector<vector<string>>();
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

  gecko_driver.init("C:\\\\Users\\Moose\\Desktop\\cmder\\bin\\geckodriver --port 0");
  if(gecko_driver.pid > 0) {
    const char *gecko_addr = local_addr_from_pid(gecko_driver.pid);
    if(gecko_addr) {
      this->driver = gecko_driver;
      this->local_addr = gecko_addr;
      if(this->session = curl_easy_init()) {
        status = 0;
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