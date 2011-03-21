#include "ComputeDiracThread.h"

//Moose Includes
#include "Moose.h"
#include "ParallelUniqueId.h"
#include "DiracKernel.h"
#include "Problem.h"
#include "NonlinearSystem.h"
#include "MooseVariable.h"

// libmesh includes
#include "threads.h"

ComputeDiracThread::ComputeDiracThread(Problem & problem,
                                       NonlinearSystem & system,
                                       NumericVector<Number> * residual,
                                       SparseMatrix<Number> * jacobian) :
    ThreadedElementLoop<DistElemRange>(problem, system),
    _residual(residual),
    _jacobian(jacobian),
    _sys(system)
{}
 
// Splitting Constructor
ComputeDiracThread::ComputeDiracThread(ComputeDiracThread & x, Threads::split split) :
    ThreadedElementLoop<DistElemRange>(x, split),
    _residual(x._residual),
    _jacobian(x._jacobian),
    _sys(x._sys)
{}

void
ComputeDiracThread::preElement(const Elem * /*elem*/)
{
  _vars.clear();
}
  
void
ComputeDiracThread::onElement(const Elem *elem)
{
  bool has_dirac_kernels_on_elem = _problem.reinitDirac(elem, _tid);

  if(has_dirac_kernels_on_elem)
  {
    DiracKernelIterator dirac_kernel_begin = _sys._dirac_kernels[_tid].diracKernelsBegin();
    DiracKernelIterator dirac_kernel_end = _sys._dirac_kernels[_tid].diracKernelsEnd();
    DiracKernelIterator dirac_kernel_it = dirac_kernel_begin;

    for(dirac_kernel_it=dirac_kernel_begin;dirac_kernel_it!=dirac_kernel_end;++dirac_kernel_it)
    {
      DiracKernel * dirac = *dirac_kernel_it;
    
      if(dirac->hasPointsOnElem(elem))
      {
        _vars.insert(&dirac->variable());

        if(_residual)
          dirac->computeResidual();
        else
          dirac->computeJacobian();
      }
    }
  }
}

void
ComputeDiracThread::postElement(const Elem * /*elem*/)
{
  for (std::set<MooseVariable *>::iterator it = _vars.begin(); it != _vars.end(); ++it)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    if(_residual)
      (*it)->add(*_residual);
    else
      (*it)->add(*_jacobian);
  }
}

void
ComputeDiracThread::join(const ComputeDiracThread & /*y*/)
{
}

