#include "Logger.h"

Logger::Logger() : _warnings_are_errors(false), _n_errors(0), _n_warnings(0) {}

Logger::~Logger()
{
  for (auto it : _msgs)
    delete it;
}

void
Logger::print()
{
  // errors
  if (_n_errors > 0)
  {
    printMessage(COLOR_RED, _n_errors, " error", (_n_errors > 1 ? "s" : ""), ":");
    for (auto it : _msgs)
      if (it->_type == ERROR)
        printMessage(COLOR_RED, "  - ", it->_msg);
    printMessage();
  }

  // warnings
  if (_n_warnings > 0)
  {
    printMessage(COLOR_YELLOW, _n_warnings, " warning", (_n_warnings > 1 ? "s" : ""), ":");
    for (auto it : _msgs)
      if (it->_type == WARNING)
        printMessage(COLOR_YELLOW, "  - ", it->_msg);
    printMessage();
  }
}

bool
Logger::isEmpty()
{
  return (_n_errors == 0) && (_n_warnings == 0);
}

unsigned int
Logger::getNumberOfErrors()
{
  return _n_errors;
}

unsigned int
Logger::getNumberOfWarnings()
{
  return _n_warnings;
}

void
Logger::printMessage()
{
  Moose::err << COLOR_DEFAULT << std::endl;
}
