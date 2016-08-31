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

#include "ComputeJacobianThread.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "TimeDerivative.h"
#include "IntegratedBC.h"
#include "DGKernel.h"
#include "InterfaceKernel.h"
#include "KernelWarehouse.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeJacobianThread::ComputeJacobianThread(FEProblem & fe_problem, NonlinearSystem & sys, SparseMatrix<Number> & jacobian) :
    ThreadedElementLoop<ConstElemRange>(fe_problem, sys),
    _jacobian(jacobian),
    _sys(sys),
    _num_cached(0),
    _integrated_bcs(sys.getIntegratedBCWarehouse()),
    _dg_kernels(sys.getDGKernelWarehouse()),
    _interface_kernels(sys.getInterfaceKernelWarehouse()),
    _kernels(sys.getKernelWarehouse())
{
}

// Splitting Constructor
ComputeJacobianThread::ComputeJacobianThread(ComputeJacobianThread & x, Threads::split split) :
    ThreadedElementLoop<ConstElemRange>(x, split),
    _jacobian(x._jacobian),
    _sys(x._sys),
    _num_cached(x._num_cached),
    _integrated_bcs(x._integrated_bcs),
    _dg_kernels(x._dg_kernels),
    _interface_kernels(x._interface_kernels),
    _kernels(x._kernels)
{
}

ComputeJacobianThread::~ComputeJacobianThread()
{
}

void
ComputeJacobianThread::computeJacobian()
{
  if (_kernels.hasActiveBlockObjects(_subdomain, _tid))
  {
    const std::vector<MooseSharedPointer<KernelBase> > & kernels = _kernels.getActiveBlockObjects(_subdomain, _tid);
    for (const auto & kernel : kernels)
      if (kernel->isImplicit())
      {
        kernel->subProblem().prepareShapes(kernel->variable().number(), _tid);
        kernel->computeJacobian();
      }
  }
}

void
ComputeJacobianThread::computeFaceJacobian(BoundaryID bnd_id)
{
  const std::vector<MooseSharedPointer<IntegratedBC> > & bcs = _integrated_bcs.getActiveBoundaryObjects(bnd_id, _tid);
  for (const auto & bc : bcs)
    if (bc->shouldApply() && bc->isImplicit())
    {
      bc->subProblem().prepareFaceShapes(bc->variable().number(), _tid);
      bc->computeJacobian();
    }
}

void
ComputeJacobianThread::computeInternalFaceJacobian(const Elem * neighbor)
{
  // No need to call hasActiveObjects, this is done in the calling method (see onInternalSide)
  const std::vector<MooseSharedPointer<DGKernel> > & dgks = _dg_kernels.getActiveBlockObjects(_subdomain, _tid);
  for (const auto & dg : dgks)
    if (dg->isImplicit())
    {
      dg->subProblem().prepareFaceShapes(dg->variable().number(), _tid);
      dg->subProblem().prepareNeighborShapes(dg->variable().number(), _tid);
      if (dg->hasBlocks(neighbor->subdomain_id()))
        dg->computeJacobian();
    }
}

void
ComputeJacobianThread::computeInternalInterFaceJacobian(BoundaryID bnd_id)
{
  // No need to call hasActiveObjects, this is done in the calling method (see onInterface)
  const std::vector<MooseSharedPointer<InterfaceKernel> > & intks = _interface_kernels.getActiveBoundaryObjects(bnd_id, _tid);
  for (const auto & intk : intks)
    if (intk->isImplicit())
    {
      intk->subProblem().prepareFaceShapes(intk->variable().number(), _tid);
      intk->subProblem().prepareNeighborShapes(intk->neighborVariable().number(), _tid);
      intk->computeJacobian();
    }
}

void
ComputeJacobianThread::subdomainChanged()
{
  _fe_problem.subdomainSetup(_subdomain, _tid);

  // Update variable Dependencies
  std::set<MooseVariableBase *> needed_moose_vars;
  _kernels.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _integrated_bcs.updateBoundaryVariableDependency(needed_moose_vars, _tid);
  _dg_kernels.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _interface_kernels.updateBoundaryVariableDependency(needed_moose_vars, _tid);

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
ComputeJacobianThread::onElement(const Elem *elem)
{
  _fe_problem.prepare(elem, _tid);

  _fe_problem.reinitElem(elem, _tid);

  _fe_problem.reinitMaterials(_subdomain, _tid);
  if (_sys.getScalarVariables(_tid).size() > 0)
    _fe_problem.reinitOffDiagScalars(_tid);

  computeJacobian();

  _fe_problem.swapBackMaterials(_tid);
}

void
ComputeJacobianThread::onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id)
{
  if (_integrated_bcs.hasActiveBoundaryObjects(bnd_id, _tid))
  {
    _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);

    _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
    _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

    // Set the active boundary id so that BoundaryRestrictable::_boundary_id is correct
    _fe_problem.setCurrentBoundaryID(bnd_id);

    computeFaceJacobian(bnd_id);

    // Set the active boundary to invalid
    _fe_problem.setCurrentBoundaryID(Moose::INVALID_BOUNDARY_ID);

    _fe_problem.swapBackMaterialsFace(_tid);
  }
}

void
ComputeJacobianThread::onInternalSide(const Elem *elem, unsigned int side)
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

      computeInternalFaceJacobian(neighbor);

      _fe_problem.swapBackMaterialsFace(_tid);
      _fe_problem.swapBackMaterialsNeighbor(_tid);

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.addJacobianNeighbor(_jacobian, _tid);
      }
    }
  }
}

void
ComputeJacobianThread::onInterface(const Elem *elem, unsigned int side, BoundaryID bnd_id)
{
  if (_interface_kernels.hasActiveBoundaryObjects(bnd_id, _tid))
  {
    // Pointer to the neighbor we are currently working on.
    const Elem * neighbor = elem->neighbor(side);

    if (neighbor->active())
    {
      _fe_problem.reinitNeighbor(elem, side, _tid);

      _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
      _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

      computeInternalInterFaceJacobian(bnd_id);

      _fe_problem.swapBackMaterialsFace(_tid);
      _fe_problem.swapBackMaterialsNeighbor(_tid);

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.addJacobianNeighbor(_jacobian, _tid);
      }
    }
  }
}

void
ComputeJacobianThread::postElement(const Elem * /*elem*/)
{
  _fe_problem.cacheJacobian(_tid);
  _num_cached++;

  if (_num_cached % 20 == 0)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _fe_problem.addCachedJacobian(_jacobian, _tid);
  }
}

void
ComputeJacobianThread::post()
{
  _fe_problem.clearActiveElementalMooseVariables(_tid);
}

void ComputeJacobianThread::join(const ComputeJacobianThread & /*y*/)
{
}
