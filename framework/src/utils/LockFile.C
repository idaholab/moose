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

#include "LockFile.h"
#include "MooseError.h"
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

LockFile::LockFile(const std::string & filename, bool do_lock)
  : _do_lock(do_lock), _fd(-1), _filename(filename)
{
  if (_do_lock)
  {
    _fd = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
    if (_fd == -1)
      mooseError("Failed to open file", filename);
    if (flock(_fd, LOCK_EX) != 0)
      mooseWarning("Failed to lock file ", filename);
  }
}

LockFile::~LockFile()
{
  if (_do_lock)
  {
    if (flock(_fd, LOCK_UN) != 0)
      mooseWarning("Failed to unlock file ", _filename);
    close(_fd);
  }
}
