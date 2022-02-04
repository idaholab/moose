//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>

/**
 * Gets an exclusive lock on a file.
 * This uses RAII to obtain the lock in the constructor and release
 * the lock in the destructor.
 * Additionally, to allow for easier use as a stack variable, an optional
 * bool is allowed to specify that no locking is actually done. This is useful
 * for the case where only certain processors need to obtain the lock.
 */
class LockFile
{
public:
  LockFile(const std::string & filename, bool do_lock = true);
  ~LockFile();

protected:
  const bool _do_lock;
  int _fd;
  const std::string _filename;
};
