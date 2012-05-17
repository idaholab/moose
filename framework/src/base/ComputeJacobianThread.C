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

// libmesh includes
#include "threads.h"

ComputeJacobianThread::ComputeJacobianThread(FEProblem & fe_problem, NonlinearSystem & sys, SparseMatrix<Number> & jacobian) :
    ThreadedElementLoop<ConstElemRange>(fe_problem, sys),
    _jacobian(jacobian),
    _problem(*fe_problem.parent()),
    _fe_problem(fe_problem),
    _sys(sys)
{}

// Splitting Constructor
ComputeJacobianThread::ComputeJacobianThread(ComputeJacobianThread & x, Threads::split split) :
    ThreadedElementLoop<ConstElemRange>(x, split),
    _jacobian(x._jacobian),
    _problem(x._problem),
    _fe_problem(x._fe_problem),
    _sys(x._sys)
{}

void
ComputeJacobianThread::computeJacobian()
{
  const std::vector<Kernel *> & kernels = _sys._kernels[_tid].active();
  for (std::vector<Kernel *>::const_iterator it = kernels.begin(); it != kernels.end(); ++it)
  {
    Kernel * kernel = *it;
    kernel->subProblem().prepareShapes(kernel->variable().number(), _tid);
    kernel->computeJacobian();
  }
}

void
ComputeJacobianThread::computeFaceJacobian(BoundaryID bnd_id)
{
  std::vector<IntegratedBC *> bcs = _sys._bcs[_tid].activeIntegrated(bnd_id);
  for (std::vector<IntegratedBC *>::iterator it = bcs.begin(); it != bcs.end(); ++it)
  {
    IntegratedBC * bc = *it;
    if (bc->shouldApply())
    {
      bc->subProblem().prepareFaceShapes(bc->variable().number(), _tid);
      bc->computeJacobian();
    }
  }
}

void
ComputeJacobianThread::computeInternalFaceJacobian()
{
  std::vector<DGKernel *> dgks = _sys._dg_kernels[_tid].active();
  for (std::vector<DGKernel *>::iterator it = dgks.begin(); it != dgks.end(); ++it)
  {
    DGKernel * dg = *it;
    dg->subProblem().prepareNeighborShapes(dg->variable().number(), _tid);
    dg->computeJacobian();
  }
}


void
ComputeJacobianThread::onElement(const Elem *elem)
{
  _problem.prepare(elem, _tid);
  _problem.reinitElem(elem, _tid);

  SubdomainID subdomain = elem->subdomain_id();
  if (subdomain != _subdomain)
  {
    _problem.subdomainSetup(subdomain, _tid);
    _sys._kernels[_tid].updateActiveKernels(subdomain);
  }

  _problem.reinitMaterials(subdomain, _tid);

  computeJacobian();
}

void
ComputeJacobianThread::onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id)
{
  std::vector<IntegratedBC *> bcs = _sys._bcs[_tid].activeIntegrated(bnd_id);
  if (bcs.size() > 0)
  {
    _problem.reinitElemFace(elem, side, bnd_id, _tid);

    unsigned int subdomain = elem->subdomain_id();
    if (subdomain != _subdomain)
      _problem.subdomainSetupSide(subdomain, _tid);

    _problem.reinitMaterialsFace(elem->subdomain_id(), side, _tid);

    computeFaceJacobian(bnd_id);
  }
}

void
ComputeJacobianThread::onInternalSide(const Elem *elem, unsigned int side)
{
  // Pointer to the neighbor we are currently working on.
  const Elem * neighbor = elem->neighbor(side);

  // Get the global id of the element and the neighbor
  const unsigned int elem_id = elem->id();
  const unsigned int neighbor_id = neighbor->id();

  if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) || (neighbor->level() < elem->level()))
  {
    std::vector<DGKernel *> dgks = _sys._dg_kernels[_tid].active();
    if (dgks.size() > 0)
    {
      _problem.reinitNeighbor(elem, side, _tid);

      _problem.reinitMaterialsFace(elem->subdomain_id(), side, _tid);
      _problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), side, _tid);

      computeInternalFaceJacobian();

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _problem.addJacobianNeighbor(_jacobian, _tid);
      }
    }
  }
}

void
ComputeJacobianThread::postElement(const Elem * /*elem*/)
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  _problem.addJacobian(_jacobian, _tid);
}

void ComputeJacobianThread::join(const ComputeJacobianThread & /*y*/)
{
}
