//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Logger.h"

Logger::Logger() : _n_errors(0), _n_warnings(0) {}

Logger::~Logger()
{
  for (auto && it : _msgs)
    delete it;
}

void
Logger::emitLoggedErrors() const
{
  if (_n_errors > 0)
  {
    std::ostringstream oss;
    oss << "The following errors were encountered:\n";
    for (const auto & msg_it : _msgs)
      if (msg_it->_type == ERROR)
        oss << "  - " << msg_it->_msg << "\n";
    mooseError(oss.str());
  }
}

void
Logger::emitLoggedWarnings() const
{
  if (_n_warnings > 0)
  {
    std::ostringstream oss;
    oss << "The following warnings were encountered:\n";
    for (const auto & msg_it : _msgs)
      if (msg_it->_type == WARNING)
        oss << "  - " << msg_it->_msg << "\n";
    mooseWarning(oss.str());
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
