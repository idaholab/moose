//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MEMORYUSAGEREPORTER_H
#define MEMORYUSAGEREPORTER_H

#include "Moose.h"
#include "MooseObject.h"

/**
 * Mix-in class for querying memory metrics used by MemoryUsage and VectorMemoryUsage
 */
class MemoryUsageReporter
{
public:
  MemoryUsageReporter(const MooseObject * moose_object);

protected:
  /// hardware IDs for each MPI rank (valid on rank zero only)
  std::vector<unsigned int> _hardware_id;

  /// total RAM installed in the local node
  unsigned long long _memory_total;

  /// total RAM for each hardware ID (node) (valid on rank zero only)
  std::vector<unsigned long long> _hardware_memory_total;
};

#endif // MEMORYUSAGEREPORTER_H
