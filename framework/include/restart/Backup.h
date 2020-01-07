//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// C++ includes
#include <sstream>
#include <vector>
#include <memory>

/**
 * Helper class to hold streams for Backup and Restore operations.
 */
class Backup
{
public:
  Backup();

  /**
   * Stream for holding binary blob data for the simulation.
   */
  std::stringstream _system_data;

  /**
   * Vector of streams for holding individual thread data for the simulation.
   */
  std::vector<std::unique_ptr<std::stringstream>> _restartable_data;
};

// Specializations for dataLoad and dataStore appear in DataIO.C
