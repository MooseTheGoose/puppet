#include "puppet_types.hpp"

/* Windows defines */
#if defined(_WIN32)
#include <windows.h>

/* UNIX-like defines */
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#endif

int is_prefix(const char *pre, const char *str) {
  while(*str == *pre && *str && *pre) {
    str++;
    pre++;
  }

  return !*pre;
}

int puppet_isspace(unichar_t ch) {
  int yes = 0;

  static uint16_t spaces[] = {
    /* ASCII Whitespace, non-contiguous */
    ' '

    /* 
     * Unicode space separators not in ASCII
     * and not contiguous
     */
    0xA0, 0x1680, 0x202F, 0x205F, 0x3000,

    /* 
     * Paragraph and "line separator" 
     * (Unicode's line separator, not Foundations's) 
     */
    0x2028, 0x2029,

    /* Null terminator */
    0
  };

  if(ch >= '\t' && ch <= '\r' || ch >= 0x2000 && ch <= 0x2010) { 
    yes = 1; 
  } else {
    for(int i = 0; spaces[i] && !yes; i++) {
      if(ch == spaces[i]) { 
        yes = 1; 
      } 
    }
  }

  return yes;
}

unichar_t puppet_toupper(unichar_t ch) {
  if(ch >= 'a' && ch <= 'z') {
    ch += 'A' - 'a';
  }
  return ch;
}

unichar_t puppet_tolower(unichar_t ch) {
  if(ch >= 'A' && ch <= 'Z') {
    ch += 'a' - 'A';
  }
  return ch;
}

unichar_t puppet_conv_xdigit(unichar_t ch) {
  ch = puppet_toupper(ch);
  return ch >= 'A' ? (ch - 'A' + 10) : (ch - '0');
}

int puppet_isdigit(unichar_t ch) {
  return ch >= '0' && ch <= '9';
}

int puppet_isalpha(unichar_t ch) {
  ch = puppet_toupper(ch);
  return ch >= 'A' && ch <= 'Z';
}

int puppet_isiden(unichar_t ch) {
  return puppet_isalpha(ch) || ch == '_' || ch >= 0x80 && !puppet_isspace(ch);
}

int puppet_isxdigit(unichar_t ch) {
  ch = puppet_conv_xdigit(ch);
  return ch >= 0 && ch < 16;
}

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
 *
 *  TODO: At the moment, output gets redirected towards a null
 *        file. Later on, introduce an optional parameter for
 *        log files when given the name (default to null file).
 *        Obviously, don't do this for PuppetPipedProcess.
 *
 *  TODO: Along those lines, have a master logger and pass it
 *        in as a parameter to log when process or file creation
 *        fails and why. Do this for PuppetPipedProcess too.
 */

int PuppetProcess::init(const char *cmd) {
  this->pid = -1;
  char *cmd_line = new char[strlen(cmd) + 1];
  for(size_t i = 0; (cmd_line[i] = cmd[i]); i++) {}

  #if defined(_WIN32)
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  this->hproc = 0;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if(CreateProcess(0, cmd_line, 0, 0, 0, CREATE_NO_WINDOW, 0, 0, &si, &pi)) {
    this->pid = pi.dwProcessId;
    this->hproc = pi.hProcess;
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
      close(1);
      close(2);
      int fd = open("/dev/null", O_WRONLY);
      dup(fd);
      dup(fd);
      close(fd);
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
  this->process.pid = -1;
  this->output = 0;
  this->len = 0; 
  vector<char> child_out = vector<char>();
  char *cmd_line = new char[strlen(cmd) + 1];
  for(size_t i = 0; (cmd_line[i] = cmd[i]); i++) {}

  #if defined(_WIN32)
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  SECURITY_ATTRIBUTES sa;
  HANDLE read_end, write_end, write_err_end;

  this->process.hproc = 0;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  ZeroMemory(&sa, sizeof(sa));
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = 0;
  sa.bInheritHandle = TRUE;

  if(CreatePipe(&read_end, &write_end, &sa, 0)) {
    DuplicateHandle(GetCurrentProcess(), write_end, GetCurrentProcess(),
                    &write_err_end, 0, TRUE, DUPLICATE_SAME_ACCESS);
    si.hStdError = write_err_end;
    si.hStdOutput = write_end;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.dwFlags = STARTF_USESTDHANDLES;
    SetHandleInformation(read_end, HANDLE_FLAG_INHERIT, 0);

    if(CreateProcess(0, cmd_line, 0, 0, TRUE, CREATE_NO_WINDOW, 0, 0, &si, &pi)) {
      CloseHandle(pi.hThread);
      CloseHandle(write_end);
      CloseHandle(write_err_end);
      this->process.pid = pi.dwProcessId;
      this->process.hproc = pi.hProcess;

      DWORD nbytes = 1;

      while(nbytes) {
        const int BUF_SZ = 9192;
        char buffer[BUF_SZ];

        if(!ReadFile(read_end, buffer, BUF_SZ, &nbytes, 0) && GetLastError() != ERROR_BROKEN_PIPE) {
          this->process.pid = -1;
          CloseHandle(this->process.hproc);
          break;
        } else {
          for(DWORD i = 0; i < nbytes; i++) {
            child_out.push_back(buffer[i]);
          }
        }
      }
      if(this->process.pid > 0) {
        char *temp_output = new char[child_out.size() + 1];
        for(size_t i = 0; i < child_out.size(); i++) {
          temp_output[i] = child_out[i];
        }
        temp_output[child_out.size()] = 0;
        this->output = temp_output;
        this->len = child_out.size();
      }
    } else {
      CloseHandle(write_end);
      CloseHandle(write_err_end);
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
        close(2);
        dup(pipefds[1]);
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
        int nbytes = read(read_end, buffer, BUF_SZ);
        
        while(nbytes > 0) {
          for(int i = 0; i < nbytes; i++) {
            child_out.push_back(buffer[i]);
          }
          nbytes = read(read_end, buffer, BUF_SZ);
        }

        if(!nbytes) {
          char *temp_output = new char[child_out.size() + 1];
          for(int i = 0; i < child_out.size(); i++) {
            temp_output[i] = child_out[i];
          }
          temp_output[child_out.size()] = 0;
          this->process.pid = pid;
          this->output = temp_output;
          this->len = child_out.size();
        }
        close(read_end);
      }
    }
  }
  #endif

  delete[] cmd_line;
  return this->process.pid;
}

int PuppetProcess::identifiable() {
  return this->pid > 0;
}

int PuppetPipedProcess::identifiable() {
  return this->process.identifiable();
}

int PuppetProcess::murder() {
  int status = -1;
  if(this->identifiable()) {
    #if defined(_WIN32)
    HANDLE term_handle = OpenProcess(PROCESS_TERMINATE, 0, this->pid);

    if(term_handle) {
      if(TerminateProcess(term_handle, 0xFFFF)) {
        CloseHandle(this->hproc);
        this->pid = -1;
        this->hproc = 0;
        status = 0;
      }
      CloseHandle(term_handle);
    }
    #else
    if(!kill(this->pid, SIGKILL)) {
      waitpid(this->pid, 0, 0);
      this->pid = -1;
      status = 0;
    }
    #endif
  }
  return status;
}

int PuppetPipedProcess::murder() {
  return this->process.murder();
}

void PuppetProcess::wait() {
  if(this->identifiable()) {
    #if defined(_WIN32)
    HANDLE query_handle = OpenProcess(PROCESS_QUERY_INFORMATION, 0, this->pid);
    DWORD status = STILL_ACTIVE;
    while(status == STILL_ACTIVE) {
      WaitForSingleObject(query_handle, INFINITE);
      GetExitCodeProcess(query_handle, &status);
    }
    CloseHandle(query_handle);
    CloseHandle(this->hproc);
    this->hproc = 0;
    #else
    waitpid(this->pid, 0, 0);
    #endif

    this->pid = -1;
  }
}

void PuppetPipedProcess::wait() {
  this->process.wait();
}

void PuppetPipedProcess::free_output() {
  if (this->output) {
    delete[] this->output;
    this->output = 0;
    this->len = 0;
  }
}

