#ifndef PUPPET_OS_HPP
#define PUPPET_OS_HPP

#if defined(_WIN32)

#include <windows.h>
#define PUPPET_SLEEP(x) Sleep(x)

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#define PUPPET_SLEEP usleep(1000 * (x))

#endif


/*
 *  Represents background processes which
 *  runs paralell to this process.
 */
struct PuppetProcess {
  int pid;

  #if defined(_WIN32)
  HANDLE hproc;
  #endif

  int init(const char *cmd_line);
  void wait();
  int murder();
  int identifiable();
};

/*
 *  Piped process which captures stdout & stderr and 
 *  makes parent wait until that is done.
 */
struct PuppetPipedProcess {
  PuppetProcess process;
  const char *output;
  size_t len;

  int init(const char *cmd_line);
  void wait();
  int murder();
  int identifiable();
  void free_output();
};


#endif