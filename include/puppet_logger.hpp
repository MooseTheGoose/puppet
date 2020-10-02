#ifndef PUPPET_LOGGER_HPP
#define PUPPET_LOGGER_HPP

#include <cstdio>
#include <vector>
#include <string>

using std::vector;
using std::string;

extern const char *const LOGGER_LINE_SEP;
extern const char *const LOGGER_BORDER;

struct StringLogger {
  string output;

  void init();
  void append_line(const char *line);
  void free();
};

struct FileLogger {
  std::FILE *fp;

  void init(const char *fname);
  void append_line(const char *line);
  void free();
};

#endif