/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "Moose.h"
#include "ExecutablePath.h"
#include "MooseError.h"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace Moose {

std::string getExecutablePath()
{
  std::string exec_path;
  char path[1024];

#ifdef __APPLE__
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) == 0)
    exec_path = path;
  else
    mooseError("Unable to retrieve executable path");
#else //Linux with Proc
  std::ostringstream oss;
  oss << "/proc/" << getpid() << "/exe";
  int ch = readlink(oss.str().c_str(), path, 1024);
  if (ch != -1)
  {
    path[ch] = 0;
    exec_path = path;
  }
#endif

  // Now strip off the exeuctable to get the PATH
  std::string::size_type t = exec_path.find_last_of("/");
  exec_path = exec_path.substr(0, t) + "/";

  return exec_path;
}

} // Namespace MOOSE

