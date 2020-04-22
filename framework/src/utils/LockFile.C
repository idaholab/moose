//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LockFile.h"
#include "MooseError.h"
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

LockFile::LockFile(const std::string & filename, bool do_lock)
  : _do_lock(do_lock), _fd(-1), _filename(filename)
{
// for now just do not do any locking on Windows
#ifndef __WIN32__
  if (_do_lock)
  {
    _fd = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
    if (_fd == -1)
      mooseError("Failed to open file", filename);
    if (flock(_fd, LOCK_EX) != 0)
      mooseWarning("Failed to lock file ", filename);
  }
#endif
}

LockFile::~LockFile()
{
#ifndef __WIN32__
  if (_do_lock)
  {
    if (flock(_fd, LOCK_UN) != 0)
      mooseWarning("Failed to unlock file ", _filename);
    close(_fd);
  }
#endif
}
