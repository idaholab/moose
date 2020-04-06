//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseObject.h"
#include "libmesh/communicator.h"

/**
 * Mix-in class for querying memory metrics used by MemoryUsage and VectorMemoryUsage
 */
class MemoryUsageReporter
{
public:
  MemoryUsageReporter(const MooseObject * moose_object);

protected:
  /// communicator for this object
  const Parallel::Communicator & _mur_communicator;

  /// this objects rank
  processor_id_type _my_rank;

  /// number of ranks in the object's communicator
  processor_id_type _nrank;

  /// hardware IDs for each MPI rank (valid on rank zero only)
  const std::vector<unsigned int> & _hardware_id;

  /// total RAM installed in the local node
  unsigned long long _memory_total;

  /// total RAM for each hardware ID (node) (valid on rank zero only)
  std::vector<unsigned long long> _hardware_memory_total;

private:
  /// Use a share memory type communicator split (MPI3)
  void sharedMemoryRanksBySplitCommunicator();

  /// Identify hardware by MPI processor name
  void sharedMemoryRanksByProcessorname();
};
