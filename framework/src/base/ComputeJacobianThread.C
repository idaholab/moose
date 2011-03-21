#include "ComputeJacobianThread.h"
#include "NonlinearSystem.h"
#include "Problem.h"

// libmesh includes
#include "threads.h"

ComputeJacobianThread::ComputeJacobianThread(Problem & problem, NonlinearSystem & sys, SparseMatrix<Number> & jacobian) :
  ThreadedElementLoop<ConstElemRange>(problem, sys),
  _jacobian(jacobian),
  _sys(sys),
  _problem(problem)
{}

// Splitting Constructor
ComputeJacobianThread::ComputeJacobianThread(ComputeJacobianThread & x, Threads::split split) :
  ThreadedElementLoop<ConstElemRange>(x, split),
  _jacobian(x._jacobian),
  _sys(x._sys),
  _problem(x._problem)
{}

void
ComputeJacobianThread::preElement(const Elem *elem)
{
  _vars.clear();
  _problem.prepare(elem, _tid);
}

void
ComputeJacobianThread::onElement(const Elem *elem)
{
  unsigned int cur_subdomain = elem->subdomain_id();
  
  _problem.reinitElem(elem, _tid);
  _problem.reinitMaterials(cur_subdomain, _tid);

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
    (*kernel_it)->computeJacobian(0, 0);
    _vars.insert(&(*kernel_it)->variable());
  }
}

void
ComputeJacobianThread::onDomainChanged(short int subdomain)
{
  _problem.subdomainSetup(subdomain, _tid);
  _sys._kernels[_tid].updateActiveKernels(_problem.time(), _problem.dt(), subdomain);
}

void
ComputeJacobianThread::onBoundary(const Elem *elem, unsigned int side, short int bnd_id)
{
  _problem.reinitElemFace(elem, side, bnd_id, _tid);
  _problem.reinitMaterialsFace(elem->subdomain_id(), side, _tid);
  for (std::vector<IntegratedBC *>::iterator it = _sys._bcs[_tid].getBCs(bnd_id).begin(); it != _sys._bcs[_tid].getBCs(bnd_id).end(); ++it)
  {
    (*it)->computeJacobian(0, 0);
    _vars.insert(&(*it)->variable());
  }
}

void
ComputeJacobianThread::postElement(const Elem * /*elem*/)
{
  for (std::set<MooseVariable *>::iterator it = _vars.begin(); it != _vars.end(); ++it)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    (*it)->add(_jacobian);
  }
}

void ComputeJacobianThread::join(const ComputeJacobianThread & /*y*/)
{
}
