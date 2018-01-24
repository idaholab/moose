//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InitialConditionWarehouse.h"

// MOOSE includes
#include "InitialCondition.h"
#include "MooseVariable.h"

InitialConditionWarehouse::InitialConditionWarehouse()
  : MooseObjectWarehouseBase<InitialCondition>(),
    _boundary_ics(libMesh::n_threads()),
    _block_ics(libMesh::n_threads())
{
}

void
InitialConditionWarehouse::initialSetup(THREAD_ID tid)
{
  MooseObjectWarehouseBase<InitialCondition>::sort(tid);
  for (const auto & ic : _active_objects[tid])
    ic->initialSetup();
}

void
InitialConditionWarehouse::addObject(std::shared_ptr<InitialCondition> object, THREAD_ID tid)
{
  // Check that when object is boundary restricted that the variable is nodal
  const MooseVariable & var = object->variable();

  // Boundary Restricted
  if (object->boundaryRestricted())
  {
    if (!var.isNodal())
      mooseError("You are trying to set a boundary restricted variable on non-nodal variable. That "
                 "is not allowed.");

    std::map<std::string, std::set<BoundaryID>>::const_iterator iter =
        _boundary_ics[tid].find(var.name());
    if (iter != _boundary_ics[tid].end() && object->hasBoundary(iter->second))
      mooseError("The initial condition '",
                 object->name(),
                 "' is being defined on a boundary that already has an initial condition defined.");
    else
      _boundary_ics[tid][var.name()].insert(object->boundaryIDs().begin(),
                                            object->boundaryIDs().end());
  }

  // Block Restricted
  else if (object->blockRestricted())
  {
    std::map<std::string, std::set<SubdomainID>>::const_iterator iter =
        _block_ics[tid].find(var.name());
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
    std::map<std::string, std::set<SubdomainID>>::const_iterator iter =
        _block_ics[tid].find(var.name());
    if (iter != _block_ics[tid].end())
      mooseError("The initial condition '",
                 object->name(),
                 "' is being defined on a block that already has an initial condition defined.");
    else
      _block_ics[tid][var.name()].insert(Moose::ANY_BLOCK_ID);
  }

  // Add the IC to the storage
  MooseObjectWarehouseBase<InitialCondition>::addObject(object, tid);
}

std::set<std::string>
InitialConditionWarehouse::getDependObjects() const
{
  std::set<std::string> depend_objects;

  const auto & ics = getActiveObjects();
  for (const auto & ic : ics)
  {
    const auto & uo = ic->getDependObjects();
    depend_objects.insert(uo.begin(), uo.end());
  }

  return depend_objects;
}
