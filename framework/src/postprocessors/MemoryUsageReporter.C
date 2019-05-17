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
#include "MooseApp.h"

MemoryUsageReporter::MemoryUsageReporter(const MooseObject * moose_object)
  : _mur_communicator(moose_object->comm()),
    _my_rank(_mur_communicator.rank()),
    _nrank(_mur_communicator.size()),
    _hardware_id(moose_object->getMooseApp().rankMap().rankHardwareIds())
{
  // get total available ram
  _memory_total = MemoryUtils::getTotalRAM();
  if (!_memory_total)
    mooseWarning("Unable to query hardware memory size in ", moose_object->name());

  // gather all per node memory to processor zero
  std::vector<unsigned long long> memory_totals(_nrank);
  _mur_communicator.gather(0, _memory_total, memory_totals);

  // validate and store per node memory
  if (_my_rank == 0)
  {
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
}
