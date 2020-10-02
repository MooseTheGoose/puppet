#include "puppet_logger.hpp"

#if defined(_WIN32)
const char *const LOGGER_LINE_SEP = "\r\n";
#else
const char *const LOGGER_LINE_SEP = "\n"; 
#endif

const char *const LOGGER_BORDER = "------------------------------------------------";

void StringLogger::init() {
  this->output = "";
}

void StringLogger::append_line(const char *line) {
  this->output += line;
  this->output += LOGGER_LINE_SEP;
}

void StringLogger::free() {
  this->output = "";
}

/*
 *  TODO: In puppet_os, consider having a way
 *        to get the error message string so that
 *        errors can be reported cross-platform.
 *        (This is a very weak TODO)
 */
void FileLogger::init(const char *fname) {
  this->fp = 0;
  if(fname) {
    this->fp = fopen(fname, "ab");
    if(!this->fp) {
      this->append_line(LOGGER_BORDER);
      perror("FileLogger");
      this->append_line((string("FAILED TO OPEN FILE ") + fname).data());
      this->append_line("RESORTING TO STDERR");
      this->append_line(LOGGER_BORDER); 
    }
  }
}

void FileLogger::append_line(const char *line) {
  std::FILE *write = this->fp ? this->fp : stderr;
  std::fprintf(write, "%s%s", line, LOGGER_LINE_SEP);
}

void FileLogger::free() {
  if(this->fp) {
    std::fclose(this->fp);
    this->fp = 0;
  }
}

