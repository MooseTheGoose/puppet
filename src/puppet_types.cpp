#include "puppet_types.hpp"

/* Windows defines */
#if defined(_WIN32)
#include <windows.h>

/* UNIX-like defines */
#else
#include <unistd.h>
#endif

const string PUPPET_LINE_SEP = "\n";

string PuppetBigInt::to_string() {
  char *buffer = mpz_get_str(0, 10, this->bignum);
  string s = buffer;
  free(buffer);
   
  return s;
}

string PuppetFloat::to_string() {
  return std::to_string(this->num);
}

string PuppetString::to_string() {
  return "\"" + this->str + "\""; 
}

string PuppetList::to_string() {
  string buf = "[";

  for(size_t i = 0; i < this->data.size(); i++) {
	PuppetData pd = this->data[i];
    buf += (pd.to_string() + ",");
  }
  if(buf.back() == ',') {
    buf.pop_back();
  }
  buf += "]";

  return buf;
}

string PuppetObject::to_string() {
  string buf = "{";

  for(size_t i = 0; i < this->keys.size(); i++) {
    buf += (this->keys[i].to_string() + ":" + this->values[i].to_string() + ",");
  }
  if(buf.back() == ',') {
    buf.pop_back();
  }
  buf += "}";
 
  return buf;
}

string PuppetData::to_string() {
  string rep = "";

  switch(identifier) {
    case TYPE_BIGINT:
      rep = this->i->to_string();
      break;
    case TYPE_FLOAT:
      rep = this->f->to_string();
      break;
    case TYPE_STRING:
      rep = this->s->to_string();
      break;
    case TYPE_LIST:
      rep = this->arr->to_string();
      break; 
    case TYPE_OBJECT:
      rep = this->obj->to_string();
      break; 
  }

  return rep;
}

/*
 *  TODO: Command line logic for UNIX-like systems is the
 *        exact same in PuppetProcess and PuppetPipedProcess.
 *        Put it in a separate function.
 */

int PuppetProcess::init(const char *cmd) {
  this->pid = -1;  
  char *cmd_line = new char[strlen(cmd) + 1];
  for(size_t i = 0; cmd_line[i] = cmd[i]; i++) {}

  #if defined(_WIN32)
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if(CreateProcess(0, cmd_line, 0, 0, 0, CREATE_NO_WINDOW, 0, 0, &si, &pi)) {
    this->pid = pi.dwProcessId;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
  #else
  vector<char *> argv = vector<char *>();
  char *iter = cmd_line;

  while(*iter) {
     while(isspace(*iter)) {
       iter++;
     }
     if(*iter) {
       argv.push_back(iter);
       while(!isspace(*iter) && *iter) {
         iter++;
       }
       if(*iter) {
         *iter++ = 0;
       }
     }  
  }
  argv.push_back(0);

  if(argv.size() > 1) {
    int pid = fork();
    if(!pid) {
      int status = execvp(argv[0], argv.data());
      if(status == -1) {
        exit(-1);
      }
    } else if(pid > 0) {
      this->pid = pid;
    }
  }
  #endif

  delete[] cmd_line;
  return this->pid;
}

int PuppetPipedProcess::init(const char *cmd) {
  this->pid = -1;
  this->output = vector<char>();
  char *cmd_line = new char[strlen(cmd) + 1];
  for(size_t i = 0; cmd_line[i] = cmd[i]; i++) {}

  #if defined(_WIN32)
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  SECURITY_ATTRIBUTES sa;
  HANDLE read_end, write_end;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  ZeroMemory(&sa, sizeof(sa));
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = 0;
  sa.bInheritHandle = TRUE;

  if(CreatePipe(&read_end, &write_end, &sa, 0)) {
    si.hStdError = write_end;
    si.hStdOutput = write_end;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.dwFlags = STARTF_USESTDHANDLES;
    SetHandleInformation(read_end, HANDLE_FLAG_INHERIT, 0);

    if(CreateProcess(0, cmd_line, 0, 0, TRUE, CREATE_NO_WINDOW, 0, 0, &si, &pi)) {
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      CloseHandle(write_end);
      this->pid = pi.dwProcessId;

      DWORD nbytes = 1;
      vector<char> child_out = vector<char>();

      while(nbytes) {
        const int BUF_SZ = 9192;
        char buffer[BUF_SZ];

        if(!ReadFile(read_end, buffer, BUF_SZ, &nbytes, 0) && GetLastError() != ERROR_BROKEN_PIPE) {
          this->pid = -1;
          break;
        } else {
          for(DWORD i = 0; i < nbytes; i++) {
            child_out.push_back(buffer[i]);
          }
        }
      }
      this->output = child_out;
    }

    CloseHandle(read_end);
  }
  #else
  vector<char *> argv = vector<char *>();
  char *iter = cmd_line;

  while(*iter) {
    while(isspace(*iter)) {
      iter++;
    }
    if(*iter) {
      argv.push_back(iter);
      while(!isspace(*iter) && *iter) {
        iter++;
      }
      if(*iter) {
        *iter++ = 0;
      }
    }
  }
  argv.push_back(0);

  if(argv.size() > 1) {
    int pipefds[2];
     
    if(!pipe(pipefds)) {
      int pid = fork();

      if(!pid) {
        close(pipefds[0]);
        close(1);
        dup(pipefds[1]);
        close(pipefds[1]);
        int status = execvp(argv[0], argv.data());
        if(!status) {
          exit(-1);
        }
      } else if(pid > 0) {
        close(pipefds[1]);
        int read_end = pipefds[0];
        const int BUF_SZ = 9192;
        char buffer[BUF_SZ];
        vector<char> out = vector<char>();
        int nbytes = read(read_end, buffer, BUF_SZ);
        
        while(nbytes > 0) {
          for(int i = 0; i < BUF_SZ; i++) {
            out.push_back(buffer[i]);
          }
          nbytes = read(read_end, buffer, BUF_SZ);
        }

        if(!nbytes) {
          this->pid = pid;
          this->output = out;
        }
      }
    }
  }
  #endif

  delete[] cmd_line;
  return this->pid;
}

static int close_pid(int pid) {
  int status = -1;

  if(pid > 0) {
    #if defined(_WIN32)
    HANDLE proc = OpenProcess(PROCESS_TERMINATE, 0, pid);
  
    if(proc && TerminateProcess(proc, 2)) {
      status = 0;
    }
    #endif
  }

  return status;
}

int PuppetProcess::quit() {
  return close_pid(this->pid);
}

int PuppetPipedProcess::quit() {
  return close_pid(this->pid);
}

