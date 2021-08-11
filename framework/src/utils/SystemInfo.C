//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SystemInfo.h"
#include "ExecutablePath.h"
#include "MooseRevision.h" ///< This file is auto-generated and contains the revision

#include "libmesh/libmesh_config.h"

#include <sstream>
#include <sys/stat.h>
#include <iomanip>
#ifdef LIBMESH_HAVE_LOCALE
#include <locale>
#endif

SystemInfo::SystemInfo(int argc, char * argv[]) : _argc(argc), _argv(argv) {}

std::string
SystemInfo::getInfo() const
{
  std::stringstream oss;
  oss << std::left << "Framework Information:\n"
      << std::setw(25) << "MOOSE Version: " << MOOSE_REVISION << '\n'
      << std::setw(25) << "LibMesh Version: " << LIBMESH_BUILD_VERSION << '\n';
#ifdef LIBMESH_DETECTED_PETSC_VERSION_MAJOR
  oss << std::setw(25) << "PETSc Version: " << LIBMESH_DETECTED_PETSC_VERSION_MAJOR << '.'
      << LIBMESH_DETECTED_PETSC_VERSION_MINOR << '.' << LIBMESH_DETECTED_PETSC_VERSION_SUBMINOR
      << '\n';
#endif
#ifdef LIBMESH_DETECTED_SLEPC_VERSION_MAJOR
  oss << std::setw(25) << "SLEPc Version: " << LIBMESH_DETECTED_SLEPC_VERSION_MAJOR << '.'
      << LIBMESH_DETECTED_SLEPC_VERSION_MINOR << '.' << LIBMESH_DETECTED_SLEPC_VERSION_SUBMINOR
      << '\n';
#endif

  // Current Time
  oss << std::setw(25) << "Current Time: " << getTimeStamp() << "\n";

  // Executable Timestamp
  std::string executable_path = getExecutable();
  std::string executable_time = getExecutableTimeStamp(executable_path);
  if (!executable_time.empty())
    oss << std::setw(25) << "Executable Timestamp: " << executable_time << "\n";

  oss << std::endl;
  return oss.str();
}

// TODO: Update libmesh to handle this function "timestamp.h"
std::string
SystemInfo::getTimeStamp(std::time_t * time_stamp) const
{
  struct tm * tm_struct;
  std::time_t local_time;

#ifdef LIBMESH_HAVE_LOCALE
  // Create time_put "facet"
  std::locale loc;
  const std::time_put<char> & tp = std::use_facet<std::time_put<char>>(loc);

  if (!time_stamp)
  {
    // Call C-style time getting functions
    local_time = time(NULL);
    time_stamp = &local_time;
  }
  tm_struct = std::localtime(time_stamp);

  // Date will eventually be stored in this ostringstream's string
  std::ostringstream date_stream;

  // See below for documentation on the use of the
  // std::time_put::put() function
  tp.put(date_stream,        /*s*/
         date_stream,        /*str*/
         date_stream.fill(), /*fill*/
         tm_struct,          /*tm*/
         'c');               /*format*/

  // Another way to use it is to totally customize the format...
  //    char pattern[]="%d %B %Y %I:%M:%S %p";
  //    tp.put(date_stream,                /*s*/
  //     date_stream,                /*str*/
  //     date_stream.fill(),         /*fill*/
  //     tm_struct,                  /*tm*/
  //     pattern,                    /*format begin*/
  //     pattern+sizeof(pattern)-1); /*format end  */

  return date_stream.str();
#else
  // C-stye code originally found here:
  // http://people.sc.fsu.edu/~burkardt/cpp_src/timestamp/timestamp.C
  // Author: John Burkardt, 24 September 2003
  const unsigned int time_size = 40;
  char time_buffer[time_size];

  if (!time_stamp)
  {
    local_time = time(NULL);
    time_stamp = &local_time;
  }
  tm_struct = std::localtime(time_stamp);

  // No more than time_size characters will be placed into the array.  If the
  // total number of resulting characters, including the terminating
  // NUL character, is not more than time_size, strftime() returns the
  // number of characters in the array, not counting the terminating
  // NUL.  Otherwise, zero is returned and the buffer contents are
  // indeterminate.
  size_t len = strftime(time_buffer, time_size, "%c", tm_struct);

  if (len != 0)
    return std::string(time_buffer);
  else
  {
    libMesh::out << "Error formatting time buffer, returning empty string!" << std::endl;
    return std::string("");
  }

#endif // LIBMESH_HAVE_LOCALE
}

std::string
SystemInfo::getExecutable() const
{
  std::string executable(_argv[0]);
  size_t last_slash = executable.find_last_of("/");
  if (last_slash != std::string::npos)
    executable = executable.substr(last_slash + 1);
  std::string exe(Moose::getExecutablePath() + executable);
  return exe;
}

std::string
SystemInfo::getExecutableTimeStamp() const
{
  const std::string exe = getExecutable();
  return getExecutableTimeStamp(exe);
}

std::string
SystemInfo::getExecutableTimeStamp(const std::string & exe) const
{
  struct stat attrib;
  if (!stat(exe.c_str(), &attrib))
    return getTimeStamp(&(attrib.st_mtime));
  return "";
}
