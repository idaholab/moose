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

#ifndef LOCKFILE_H
#define LOCKFILE_H

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

#endif // LOCKFILE_H
