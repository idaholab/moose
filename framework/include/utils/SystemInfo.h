//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>
#include <ctime>

class SystemInfo
{
public:
  std::string getInfo() const;
  std::string getTimeStamp(std::time_t * time_stamp = NULL) const;
  std::string getExecutable() const;
  std::string getExecutableTimeStamp() const;
  std::string getExecutableTimeStamp(const std::string & exe) const;
};
