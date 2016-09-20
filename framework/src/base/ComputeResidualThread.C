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

#include "ComputeResidualThread.h"
#include "NonlinearSystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "KernelBase.h"
#include "IntegratedBC.h"
#include "DGKernel.h"
#include "InterfaceKernel.h"
#include "Material.h"
#include "TimeKernel.h"
#include "KernelWarehouse.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeResidualThread::ComputeResidualThread(FEProblem & fe_problem, NonlinearSystem & sys, Moose::KernelType type) :
    ThreadedElementLoop<ConstElemRange>(fe_problem),
    _sys(sys),
    _kernel_type(type),
    _num_cached(0),
    _integrated_bcs(sys.getIntegratedBCWarehouse()),
    _dg_kernels(sys.getDGKernelWarehouse()),
    _interface_kernels(sys.getInterfaceKernelWarehouse()),
    _kernels(sys.getKernelWarehouse()),
    _time_kernels(sys.getTimeKernelWarehouse()),
    _non_time_kernels(sys.getNonTimeKernelWarehouse())
{
}

// Splitting Constructor
ComputeResidualThread::ComputeResidualThread(ComputeResidualThread & x, Threads::split split) :
    ThreadedElementLoop<ConstElemRange>(x, split),
    _sys(x._sys),
    _kernel_type(x._kernel_type),
    _num_cached(0),
    _integrated_bcs(x._integrated_bcs),
    _dg_kernels(x._dg_kernels),
    _interface_kernels(x._interface_kernels),
    _kernels(x._kernels),
    _time_kernels(x._time_kernels),
    _non_time_kernels(x._kernels)
{
}

ComputeResidualThread::~ComputeResidualThread()
{
}

void
ComputeResidualThread::subdomainChanged()
{
  _fe_problem.subdomainSetup(_subdomain, _tid);

  // Update variable Dependencies
  std::set<MooseVariable *> needed_moose_vars;
  _kernels.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _integrated_bcs.updateBoundaryVariableDependency(needed_moose_vars, _tid);
  _dg_kernels.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _interface_kernels.updateBoundaryVariableDependency(needed_moose_vars, _tid);

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
ComputeResidualThread::onElement(const Elem *elem)
{
  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);
  _fe_problem.reinitMaterials(_subdomain, _tid);


  const MooseObjectWarehouse<KernelBase> * warehouse;
  switch (_kernel_type)
  {
  case Moose::KT_ALL:
    warehouse = &_kernels;
    break;

  case Moose::KT_TIME:
    warehouse = &_time_kernels;
    break;

  case Moose::KT_NONTIME:
    warehouse = &_non_time_kernels;
    break;
  }

  if (warehouse->hasActiveBlockObjects(_subdomain, _tid))
  {
    const std::vector<MooseSharedPointer<KernelBase> > & kernels = warehouse->getActiveBlockObjects(_subdomain, _tid);
    for (const auto & kernel : kernels)
      kernel->computeResidual();
  }

  _fe_problem.swapBackMaterials(_tid);
}

void
ComputeResidualThread::onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id)
{
  if (_integrated_bcs.hasActiveBoundaryObjects(bnd_id, _tid))
  {
    const std::vector<MooseSharedPointer<IntegratedBC> > & bcs = _integrated_bcs.getActiveBoundaryObjects(bnd_id, _tid);

    _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);


    _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
    _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

    // Set the active boundary id so that BoundaryRestrictable::_boundary_id is correct
    _fe_problem.setCurrentBoundaryID(bnd_id);

    for (const auto & bc : bcs)
    {
      if (bc->shouldApply())
        bc->computeResidual();
    }
    _fe_problem.swapBackMaterialsFace(_tid);

    // Set active boundary id to invalid
    _fe_problem.setCurrentBoundaryID(Moose::INVALID_BOUNDARY_ID);
  }
}

void
ComputeResidualThread::onInterface(const Elem *elem, unsigned int side, BoundaryID bnd_id)
{
  if (_interface_kernels.hasActiveBoundaryObjects(bnd_id, _tid))
  {

    // Pointer to the neighbor we are currently working on.
    const Elem * neighbor = elem->neighbor(side);

    if (!(neighbor->level() == elem->level()))
      mooseError("Sorry, interface kernels do not work with mesh adaptivity");

    if (neighbor->active())
    {
      _fe_problem.reinitNeighbor(elem, side, _tid);

      _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
      _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

      const std::vector<MooseSharedPointer<InterfaceKernel> > & int_ks = _interface_kernels.getActiveBoundaryObjects(bnd_id, _tid);
      for (const auto & interface_kernel : int_ks)
        interface_kernel->computeResidual();

      _fe_problem.swapBackMaterialsFace(_tid);
      _fe_problem.swapBackMaterialsNeighbor(_tid);

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.addResidualNeighbor(_tid);
      }
    }
  }
}

void
ComputeResidualThread::onInternalSide(const Elem *elem, unsigned int side)
{
  if (_dg_kernels.hasActiveBlockObjects(_subdomain, _tid))
  {
    // Pointer to the neighbor we are currently working on.
    const Elem * neighbor = elem->neighbor(side);

    // Get the global id of the element and the neighbor
    const dof_id_type
      elem_id = elem->id(),
      neighbor_id = neighbor->id();

    if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) || (neighbor->level() < elem->level()))
    {
      _fe_problem.reinitNeighbor(elem, side, _tid);

      _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
      _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

      const std::vector<MooseSharedPointer<DGKernel> > & dgks = _dg_kernels.getActiveBlockObjects(_subdomain, _tid);
      for (const auto & dg_kernel : dgks)
        if (dg_kernel->hasBlocks(neighbor->subdomain_id()))
          dg_kernel->computeResidual();

      _fe_problem.swapBackMaterialsFace(_tid);
      _fe_problem.swapBackMaterialsNeighbor(_tid);

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.addResidualNeighbor(_tid);
      }
    }
  }
}

void
ComputeResidualThread::postElement(const Elem * /*elem*/)
{
  _fe_problem.cacheResidual(_tid);
  _num_cached++;

  if (_num_cached % 20 == 0)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _fe_problem.addCachedResidual(_tid);
  }
}

void
ComputeResidualThread::post()
{
  _fe_problem.clearActiveElementalMooseVariables(_tid);
}


void
ComputeResidualThread::join(const ComputeResidualThread & /*y*/)
{
}
