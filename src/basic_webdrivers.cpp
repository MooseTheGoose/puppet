#include "basic_webdrivers.hpp"
#include <stdio.h>

BasicHeaders::BasicHeaders() {
  labels = vector<string>();
  values = vector<string>();
}

void BasicHeaders::ManipulateHeader(const string &label, const string &value) {
  for(int i = 0; i < this->labels.size(); i++) {
    if(!label.compare(this->labels[i])) {
      this->labels[i] = label;
      this->values[i] = value;
      return;
    }
  }

  this->labels.push_back(label);
  this->values.push_back(value);
}

void BasicHeaders::DeleteHeader(const string &label) {
  for(int i = 0; i < this->labels.size(); i++) {
    if(!label.compare(this->labels[i])) {
      this->labels[i] = this->labels.back();
      this->values[i] = this->values.back();
      this->labels.pop_back();
      this->values.pop_back();
      return;
    }
  }

  throw "Error: Tried to delete a header that wasn't present";
}


BasicWebdriver::BasicWebdriver() {
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  char cmd[256];
  char *portno = "4444";
  struct sockaddr_in localhost;

  /* Fire up a GeckoDriver server */

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  sprintf(cmd, "geckodriver --port %s", portno);

  if(!CreateProcessA(0, cmd, 0, 0, 0, CREATE_NEW_CONSOLE, 0, 0, &si, &pi)) {
    throw "Failed to create GeckoDriver for BasicWebdriver";
  }


  /* Fill in the sockaddr struct and save it in class */

  localhost.sin_family = AF_INET;
  localhost.sin_port = htons(4444);
  InetPtonA(AF_INET, "127.0.0.1", &localhost.sin_addr.s_addr);
  ZeroMemory(localhost.sin_zero, sizeof(localhost.sin_zero));
  *((struct sockaddr_in *)&this->driver_addr) = localhost;
  this->addr_len = sizeof(localhost);
}

vector<char> BasicWebdriver::fetch(const string &method, const string &endpoint, 
                                   const BasicHeaders &headers, const vector<char> &body) {
  SOCKET puppet = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  char recvheaders[9192];
  char content[1 << 16];
  vector<char> data;

  if(puppet == INVALID_SOCKET) {
    throw "Failed to create socket for fetch";
  }
  if(connect(puppet, (struct sockaddr *)&this->driver_addr, this->addr_len) == SOCKET_ERROR) {
    throw "Failed to connect to Webdriver server for fetch";
  }

  string reqheaders = method + " " + endpoint + " HTTP/1.1\r\n";
  int nbytes;

  for(int i = 0; i < headers.labels.size(); i++) {
    reqheaders += headers.labels[i] + ":" + headers.values[i] + "\r\n";
  }
  reqheaders += "\r\n";

  if(send(puppet, reqheaders.data(), reqheaders.size(), 0) != reqheaders.size()) {
    throw "Failed to send fetch headers";
  }

  if(send(puppet, body.data(), body.size(), 0) != body.size()) {
    throw "Failed to send fetch content";
  }

  nbytes = recv(puppet, recvheaders, 9192, 0);
  if(nbytes <= 0) {
    throw "Failed to receive headers of fetch response";
  }
  recvheaders[nbytes] = 0;
  printf("\r\n%s", recvheaders);

  nbytes = recv(puppet, content, 1 << 16, 0);
  if(nbytes <= 0) {
    throw "Failed to receive content of fetch response";
  }

  data = vector<char>();
  for(int i = 0; i < nbytes; i++) {
    data.push_back(content[i]);
  }
  closesocket(puppet);
  return data;
}
