//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Logger.h"

Logger::Logger() : _warnings_are_errors(false), _n_errors(0), _n_warnings(0) {}

Logger::~Logger()
{
  for (auto && it : _msgs)
    delete it;
}

void
Logger::print() const
{
  // errors
  if (_n_errors > 0)
  {
    printMessage(COLOR_RED, _n_errors, " error", (_n_errors > 1 ? "s" : ""), ":");
    for (auto && it : _msgs)
      if (it->_type == ERROR)
        printMessage(COLOR_RED, "  - ", it->_msg);
    printMessage();
  }

  // warnings
  if (_n_warnings > 0)
  {
    printMessage(COLOR_YELLOW, _n_warnings, " warning", (_n_warnings > 1 ? "s" : ""), ":");
    for (auto && it : _msgs)
      if (it->_type == WARNING)
        printMessage(COLOR_YELLOW, "  - ", it->_msg);
    printMessage();
  }
}

unsigned int
Logger::getNumberOfErrors() const
{
  return _n_errors;
}

unsigned int
Logger::getNumberOfWarnings() const
{
  return _n_warnings;
}

void
Logger::setWarningsAsErrors()
{
  _warnings_are_errors = true;
}

void
Logger::printMessage() const
{
  Moose::err << COLOR_DEFAULT << std::endl;
}
