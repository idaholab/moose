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
#include "MProblem.h"

// libmesh includes
#include "threads.h"

ComputeResidualThread::ComputeResidualThread(Problem & problem,
                                             NonlinearSystem & sys,
                                             NumericVector<Number> & residual) :
    ThreadedElementLoop<ConstElemRange>(problem, sys),
    _residual(residual),
    _sys(sys)
{
}

// Splitting Constructor
ComputeResidualThread::ComputeResidualThread(ComputeResidualThread & x, Threads::split split) :
    ThreadedElementLoop<ConstElemRange>(x, split),
    _residual(x._residual),
    _sys(x._sys)
{
}

void
ComputeResidualThread::onElement(const Elem *elem)
{
  _problem.prepare(elem, _tid);
  _problem.reinitElem(elem, _tid);

  unsigned int subdomain = elem->subdomain_id();
  if (subdomain != _subdomain)
  {
    _problem.subdomainSetup(subdomain, _tid);
    _sys._kernels[_tid].updateActiveKernels(_problem.time(), _problem.dt(), subdomain);
    if (_sys._doing_dg) _sys._dg_kernels[_tid].updateActiveDGKernels(_problem.time(), _problem.dt());
  }

  _problem.reinitMaterials(subdomain, _tid);

  for (std::vector<Kernel *>::const_iterator kernel_it = _sys._kernels[_tid].active().begin(); kernel_it != _sys._kernels[_tid].active().end(); ++kernel_it)
    (*kernel_it)->computeResidual();
}

void
ComputeResidualThread::onBoundary(const Elem *elem, unsigned int side, short int bnd_id)
{
  std::vector<IntegratedBC *> bcs = _sys._bcs[_tid].getBCs(bnd_id);
  if (bcs.size() > 0)
  {
    _problem.reinitElemFace(elem, side, bnd_id, _tid);

    unsigned int subdomain = elem->subdomain_id();
    if (subdomain != _subdomain)
      _problem.subdomainSetupSide(subdomain, _tid);

    _problem.reinitMaterialsFace(elem->subdomain_id(), side, _tid);

    for (std::vector<IntegratedBC *>::iterator it = bcs.begin(); it != bcs.end(); ++it)
      (*it)->computeResidual();
  }
}

void
ComputeResidualThread::onInternalSide(const Elem *elem, unsigned int side)
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
      for (std::vector<DGKernel *>::iterator it = dgks.begin(); it != dgks.end(); ++it)
      {
        DGKernel * dg = *it;
        dg->computeResidual();
      }

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _problem.addResidualNeighbor(_residual, _tid);
      }
    }
  }
}

void
ComputeResidualThread::postElement(const Elem * /*elem*/)
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  _problem.addResidual(_residual, _tid);
}

void
ComputeResidualThread::join(const ComputeResidualThread & /*y*/)
{
}
