//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankMap.h"

#include "PerfGraphInterface.h"

RankMap::RankMap(const Parallel::Communicator & comm, PerfGraph & perf_graph)
  : ParallelObject(comm),
    PerfGraphInterface(perf_graph, "RankMap"),
    _construct_timer(registerTimedSection("construct", 2))
{
  TIME_SECTION(_construct_timer);

  auto num_procs = n_processors();

  _rank_to_hardware_id.resize(num_procs);

  // This will be the world rank of the root process
  // from the shared memory communicator we're getting ready to create
  // Each process on the same node will end up with the same world_rank
  processor_id_type world_rank = 0;

#ifdef LIBMESH_HAVE_MPI
  // Create a split communicator among shared memory ranks
  MPI_Comm shmem_raw_comm;
  MPI_Comm_split_type(_communicator.get(), MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &shmem_raw_comm);
  Parallel::Communicator shmem_comm(shmem_raw_comm);

  // Broadcast the world rank of the sub group root to all processes within this communicator
  world_rank = processor_id();
  shmem_comm.broadcast(world_rank, 0);
#endif

  // Send the info to everyone
  std::vector<processor_id_type> world_ranks(num_procs);
  _communicator.allgather(world_rank, world_ranks);

  // Map of world_rank to hardware_id
  std::map<unsigned int, unsigned int> world_rank_to_hardware_id;

  // Assign a contiguous unique numerical id to each shared memory group
  unsigned int next_id = 0;

  for (MooseIndex(world_ranks) pid = 0; pid < world_ranks.size(); pid++)
  {
    auto world_rank = world_ranks[pid];

    auto it = world_rank_to_hardware_id.lower_bound(world_rank);

    unsigned int current_id = 0;

    // If we've seen this world_rank before then use its already given ID
    if (it != world_rank_to_hardware_id.end() && it->first == world_rank)
      current_id = it->second;
    else // Create the new ID
    {
      current_id = next_id++;

      world_rank_to_hardware_id.emplace_hint(it, world_rank, current_id);
    }

    _rank_to_hardware_id[pid] = current_id;

    // Side-effect insertion utilized
    _hardware_id_to_ranks[current_id].emplace_back(pid);
  }

  // Free up the memory
  MPI_Comm_free(&shmem_raw_comm);
}
