//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <string>
#include <ctime>

class SystemInfo
{
public:
  SystemInfo(int argc, char * argv[]);

  std::string getInfo() const;
  std::string getTimeStamp(std::time_t * time_stamp = NULL) const;

  int argc() const { return _argc; };
  char ** argv() const { return _argv; };

protected:
  int _argc;
  char ** _argv;
};

#endif // SYSTEMINFO_H
