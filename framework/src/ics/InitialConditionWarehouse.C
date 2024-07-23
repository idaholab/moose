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
#include "InitialConditionBase.h"
#include "MooseVariableFE.h"
#include <string>

InitialConditionWarehouse::InitialConditionWarehouse()
  : MooseObjectWarehouseBase<InitialConditionBase>(),
    _boundary_ics(libMesh::n_threads()),
    _block_ics(libMesh::n_threads())
{
}

void
InitialConditionWarehouse::initialSetup(THREAD_ID tid)
{
  MooseObjectWarehouseBase<InitialConditionBase>::sort(tid);
  for (const auto & ic : _active_objects[tid])
    ic->initialSetup();
}

void
InitialConditionWarehouse::addObject(std::shared_ptr<InitialConditionBase> object,
                                     THREAD_ID tid,
                                     bool recurse)
{
  // Check that when object is boundary restricted that the variable is nodal
  const MooseVariableFEBase & var = object->variable();
  const std::tuple<std::string, int> ic_key = std::tuple(var.name(), object->_my_state);

  // Boundary Restricted
  if (object->boundaryRestricted())
  {
    if (!var.isNodal())
      mooseError("You are trying to set a boundary restricted variable on non-nodal variable. That "
                 "is not allowed.");

    std::map<std::tuple<std::string, int>, std::set<BoundaryID>>::const_iterator iter =
        _boundary_ics[tid].find(ic_key);
    if (iter != _boundary_ics[tid].end() && object->hasBoundary(iter->second))
      mooseError("The initial condition '",
                 object->name(),
                 "' is being defined on a boundary that already has an initial condition defined "
                 "(for the variable and state combo).");
    else
      _boundary_ics[tid][ic_key].insert(object->boundaryIDs().begin(), object->boundaryIDs().end());
  }

  // Block Restricted
  else if (object->blockRestricted())
  {
    std::map<std::tuple<std::string, int>, std::set<SubdomainID>>::const_iterator iter =
        _block_ics[tid].find(ic_key);
    if (iter != _block_ics[tid].end() &&
        (object->hasBlocks(iter->second) ||
         (iter->second.find(Moose::ANY_BLOCK_ID) != iter->second.end())))
      mooseError("The initial condition '",
                 object->name(),
                 "' is being defined on a block that already has an initial condition defined (for "
                 "the variable and state combo).");
    else
      _block_ics[tid][ic_key].insert(object->blockIDs().begin(), object->blockIDs().end());
  }

  // Non-restricted
  else
  {
    std::map<std::tuple<std::string, int>, std::set<SubdomainID>>::const_iterator iter =
        _block_ics[tid].find(ic_key);
    if (iter != _block_ics[tid].end())
      mooseError("The initial condition '",
                 object->name(),
                 "' is being defined on a block that already has an initial condition defined (for "
                 "the variable and state combo).");
    else
      _block_ics[tid][ic_key].insert(Moose::ANY_BLOCK_ID);
  }

  // Add the IC to the storage
  MooseObjectWarehouseBase<InitialConditionBase>::addObject(object, tid, recurse);
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
