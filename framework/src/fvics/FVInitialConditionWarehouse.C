//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInitialConditionWarehouse.h"

// MOOSE includes
#include "FVInitialConditionBase.h"
#include "MooseVariableFE.h"

FVInitialConditionWarehouse::FVInitialConditionWarehouse()
  : MooseObjectWarehouseBase<FVInitialConditionBase>(), _block_ics(libMesh::n_threads())
{
}

void
FVInitialConditionWarehouse::initialSetup(THREAD_ID tid)
{
  MooseObjectWarehouseBase<FVInitialConditionBase>::sort(tid);
  for (const auto & ic : _active_objects[tid])
    ic->initialSetup();
}

void
FVInitialConditionWarehouse::addObject(std::shared_ptr<FVInitialConditionBase> object,
                                       THREAD_ID tid,
                                       bool recurse)
{
  // Check that when object is boundary restricted that the variable is nodal
  const auto & var = object->variable();

  // Block Restricted
  if (object->blockRestricted())
  {
    auto iter = _block_ics[tid].find(var.name());
    if (iter != _block_ics[tid].end() &&
        (object->hasBlocks(iter->second) ||
         (iter->second.find(Moose::ANY_BLOCK_ID) != iter->second.end())))
      mooseError("The initial condition '",
                 object->name(),
                 "' is being defined on a block that already has an initial condition defined.");
    else
      _block_ics[tid][var.name()].insert(object->blockIDs().begin(), object->blockIDs().end());
  }
  // Non-restricted
  else
  {
    auto iter = _block_ics[tid].find(var.name());
    if (iter != _block_ics[tid].end())
      mooseError("The initial condition '",
                 object->name(),
                 "' is being defined on a block that already has an initial condition defined.");
    else
      _block_ics[tid][var.name()].insert(Moose::ANY_BLOCK_ID);
  }

  // Add the IC to the storage
  MooseObjectWarehouseBase<FVInitialConditionBase>::addObject(object, tid, recurse);
}
