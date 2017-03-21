/****************************************************************/
/*               Do NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "InitialConditionWarehouse.h"
#include "InitialCondition.h"

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
