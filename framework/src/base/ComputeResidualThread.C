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
  _vars.clear();
  _problem.prepare(elem, _tid);
  _problem.reinitElem(elem, _tid);

  unsigned int subdomain = elem->subdomain_id();
  if (subdomain != _subdomain)
  {
    _problem.subdomainSetup(subdomain, _tid);
    _sys._kernels[_tid].updateActiveKernels(_problem.time(), _problem.dt(), subdomain);
  }

  _problem.reinitMaterials(subdomain, _tid);

  //Stabilizers
  StabilizerIterator stabilizer_begin = _sys._stabilizers[_tid].activeStabilizersBegin();
  StabilizerIterator stabilizer_end = _sys._stabilizers[_tid].activeStabilizersEnd();
  StabilizerIterator stabilizer_it = stabilizer_begin;
  for(stabilizer_it=stabilizer_begin;stabilizer_it!=stabilizer_end;stabilizer_it++)
    (*stabilizer_it)->computeTestFunctions();

  KernelIterator kernel_begin = _sys._kernels[_tid].activeKernelsBegin();
  KernelIterator kernel_end = _sys._kernels[_tid].activeKernelsEnd();
  KernelIterator kernel_it = kernel_begin;

  for (kernel_it = kernel_begin; kernel_it != kernel_end; ++kernel_it)
  {
    (*kernel_it)->computeResidual();
    _vars.insert(&(*kernel_it)->variable());
  }
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
    {
      (*it)->computeResidual();
      _vars.insert(&(*it)->variable());
    }
  }
}

void
ComputeResidualThread::postElement(const Elem * /*elem*/)
{
  for (std::set<MooseVariable *>::iterator it = _vars.begin(); it != _vars.end(); ++it)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    (*it)->add(_residual);
  }
}

void
ComputeResidualThread::join(const ComputeResidualThread & /*y*/)
{
}
