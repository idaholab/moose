/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
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

// MOOSE includes
#include "AuxKernelWarehouse.h"


AuxKernelWarehouse::AuxKernelWarehouse() :
    WarehouseBase<AuxKernel>(/*threaded=*/true)
{
}

void
AuxKernelWarehouse::addObject(MooseSharedPointer<AuxKernel> object, THREAD_ID tid)
{
  _all.addObject(object, tid);

  if (object->isNodal())
    _nodal.addObject(object, tid);
  else
    _elemental.addObject(object, tid);
}

const MooseObjectStorage<AuxKernel> &
AuxKernelWarehouse::getStorage(AuxKernelType kernel_type, ExecFlagType exec_type)
{
  switch (kernel_type)
  {
  case NODAL:
    return _nodal[exec_type];
    break;
  case ELEMENTAL:
    return _elemental[exec_type];
    break;
  default:
    break;
  }
  return _all;
}


void
AuxKernelWarehouse::initialSetup(THREAD_ID tid)
{
  _all.sort(tid);
  _nodal.sort(tid);
  _elemental.sort(tid);
  _all.initialSetup(tid);
}


void
AuxKernelWarehouse::timestepSetup(THREAD_ID tid)
{
  _all.timestepSetup(tid);
}


void
AuxKernelWarehouse::subdomainSetup(THREAD_ID tid)
{
  _all.subdomainSetup(tid);
}


void
AuxKernelWarehouse::jacobianSetup(THREAD_ID tid)
{
  _nodal[EXEC_NONLINEAR].jacobianSetup(tid);
  _elemental[EXEC_NONLINEAR].jacobianSetup(tid);
}


void
AuxKernelWarehouse::residualSetup(THREAD_ID tid)
{
  _nodal[EXEC_LINEAR].residualSetup(tid);
  _elemental[EXEC_LINEAR].residualSetup(tid);
}


void
AuxKernelWarehouse::updateActive(THREAD_ID tid)
{
  _all.updateActive(tid);
  _nodal.updateActive(tid);
  _elemental.updateActive(tid);
}

bool
AuxKernelWarehouse::hasActiveBoundaryObjects(BoundaryID id, THREAD_ID tid)
{
  bool has_active = false;
  std::map<ExecFlagType, MooseObjectStorage<AuxKernel> >::iterator iter;
  for (iter = _elemental.begin(); iter != _elemental.end(); ++iter)
    has_active |= iter->second.hasActiveBoundaryObjects(id, tid);
  return has_active;
}
