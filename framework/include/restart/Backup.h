//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BACKUP_H
#define BACKUP_H

// C++ includes
#include <sstream>
#include <vector>

/**
 * Helper class to hold streams for Backup and Restore operations.
 */
class Backup
{
public:
  Backup();

  ~Backup();

  std::stringstream _system_data;

  std::vector<std::stringstream *> _restartable_data;
};

// Specializations for dataLoad and dataStore appear in DataIO.C

#endif /* BACKUP_H */
