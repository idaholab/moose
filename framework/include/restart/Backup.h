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

#ifndef BACKUP_H
#define BACKUP_H

// C++ includes
#include <sstream>
#include <list>
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
