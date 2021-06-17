//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Moose.h"
#include "ExecutablePath.h"
#include "MooseError.h"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include <unistd.h>

namespace Moose
{

std::string
getExec()
{
  std::string exec_path;
  char path[1024];

#if defined(__APPLE__)
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) == 0)
    exec_path = path;
  else
    mooseError("Unable to retrieve executable path");
#elif defined(__WIN32__)
  return "./";
#else // Linux with Proc
  std::ostringstream oss;
  oss << "/proc/" << getpid() << "/exe";
  int ch = readlink(oss.str().c_str(), path, 1024);
  if (ch != -1)
  {
    path[ch] = 0;
    exec_path = path;
  }
#endif
  return exec_path;
}

std::string
getExecutablePath()
{
  auto exec_path = getExec();
  // strip off the exeuctable to get the PATH
  std::string::size_type t = exec_path.find_last_of("/");
  return exec_path.substr(0, t) + "/";
}

std::string
getExecutableName()
{
  auto name = getExec();
  // strip off the path to get the name
  std::string::size_type start = name.find_last_of("/");
  if (start == std::string::npos)
    start = 0;
  return name.substr(start, std::string::npos);
}

} // Namespace MOOSE
