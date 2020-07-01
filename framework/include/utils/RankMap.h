//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PerfGraphInterface.h"

#include "libmesh/parallel_object.h"

/**
 * Builds lists and maps that help in knowing which physical hardware nodes each rank is on.
 *
 * Note: large chunks of this code were originally committed by @dschwen in PR #12351
 *
 * https://github.com/idaholab/moose/pull/12351
 */
class RankMap : ParallelObject, PerfGraphInterface
{
public:
  /**
   * Constructs and fills the map
   */
  RankMap(const Parallel::Communicator & comm, PerfGraph & perf_graph);

  /**
   * Returns the "hardware ID" (a unique ID given to each physical compute node in the job)
   * for a given processor ID (rank)
   */
  unsigned int hardwareID(processor_id_type pid) const
  {
    mooseAssert(pid < _communicator.size(), "PID out of range");
    return _rank_to_hardware_id[pid];
  }

  /**
   * Returns the ranks that are on the given hardwareID (phsical node in the job)
   */
  const std::vector<processor_id_type> & ranks(unsigned int hardware_id) const
  {
    auto item = _hardware_id_to_ranks.find(hardware_id);
    if (item == _hardware_id_to_ranks.end())
      mooseError("hardware_id not found");

    return item->second;
  }

  /**
   * Vector containing the hardware ID for each PID
   */
  const std::vector<unsigned int> & rankHardwareIds() const { return _rank_to_hardware_id; }

protected:
  /// Map of hardware_id -> ranks on that node
  std::unordered_map<unsigned int, std::vector<processor_id_type>> _hardware_id_to_ranks;

  /// Each entry corresponds to the hardware_id for that PID
  std::vector<unsigned int> _rank_to_hardware_id;
};
