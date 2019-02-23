//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MemoryUsageReporter.h"
#include "MemoryUtils.h"

MemoryUsageReporter::MemoryUsageReporter(const MooseObject * moose_object)
  : _mur_communicator(moose_object->comm()),
    _my_rank(_mur_communicator.rank()),
    _nrank(_mur_communicator.size()),
    _hardware_id(_nrank)
{
  // get total available ram
  _memory_total = MemoryUtils::getTotalRAM();
  if (!_memory_total)
    mooseWarning("Unable to query hardware memory size in ", moose_object->name());

  // gather all per node memory to processor zero
  std::vector<unsigned long long> memory_totals(_nrank);
  _mur_communicator.gather(0, _memory_total, memory_totals);

  sharedMemoryRanksBySplitCommunicator();

  // validate and store per node memory
  if (_my_rank == 0)
    for (std::size_t i = 0; i < _nrank; ++i)
    {
      auto id = _hardware_id[i];
      if (id == _hardware_memory_total.size())
      {
        _hardware_memory_total.resize(id + 1);
        _hardware_memory_total[id] = memory_totals[i];
      }
      else if (_hardware_memory_total[id] != memory_totals[i])
        mooseWarning("Inconsistent total memory reported by ranks on the same hardware node in ",
                     moose_object->name());
    }
}

void
MemoryUsageReporter::sharedMemoryRanksBySplitCommunicator()
{
  // figure out which ranks share memory
  processor_id_type world_rank = 0;
#ifdef LIBMESH_HAVE_MPI
  // create a split communicator among shared memory ranks
  MPI_Comm shmem_raw_comm;
  MPI_Comm_split_type(
      _mur_communicator.get(), MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &shmem_raw_comm);
  Parallel::Communicator shmem_comm(shmem_raw_comm);

  // broadcast the world rank of the sub group root
  world_rank = _my_rank;
  shmem_comm.broadcast(world_rank, 0);

  MPI_Comm_free(&shmem_raw_comm);
#endif
  std::vector<processor_id_type> world_ranks(_nrank);
  _mur_communicator.gather(0, world_rank, world_ranks);

  // assign a contiguous unique numerical id to each shared memory group on processor zero
  unsigned int id = 0;
  processor_id_type last = world_ranks[0];
  if (_my_rank == 0)
    for (std::size_t i = 0; i < _nrank; ++i)
    {
      if (world_ranks[i] != last)
      {
        last = world_ranks[i];
        id++;
      }
      _hardware_id[i] = id;
    }
}

void
MemoryUsageReporter::sharedMemoryRanksByProcessorname()
{
  // get processor names and assign a unique number to each piece of hardware
  std::string processor_name = MemoryUtils::getMPIProcessorName();

  // gather all names at processor zero
  std::vector<std::string> processor_names(_nrank);
  _mur_communicator.gather(0, processor_name, processor_names);

  // assign a unique numerical id to them on processor zero
  unsigned int id = 0;
  if (_my_rank == 0)
  {
    // map to assign an id to each processor name string
    std::map<std::string, unsigned int> hardware_id_map;
    for (std::size_t i = 0; i < _nrank; ++i)
    {
      // generate or look up unique ID for the current processor name
      auto it = hardware_id_map.lower_bound(processor_names[i]);
      if (it == hardware_id_map.end() || it->first != processor_names[i])
        it = hardware_id_map.emplace_hint(it, processor_names[i], id++);
      _hardware_id[i] = it->second;
    }
  }
}
