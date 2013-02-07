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

#include "ComputeDiracThread.h"

//Moose Includes
#include "ParallelUniqueId.h"
#include "DiracKernel.h"
#include "Problem.h"
#include "NonlinearSystem.h"
#include "MooseVariable.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeDiracThread::ComputeDiracThread(FEProblem & feproblem,
                                       NonlinearSystem & system,
                                       NumericVector<Number> * residual,
                                       SparseMatrix<Number> * jacobian) :
    ThreadedElementLoop<DistElemRange>(feproblem, system),
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

ComputeDiracThread::~ComputeDiracThread()
{
}

void
ComputeDiracThread::pre()
{
  // Force TID=0 because we run this object _NON THREADED_
  // Take this out if we ever get Dirac's working with threads!
  _tid = 0;
}

void
ComputeDiracThread::onElement(const Elem * elem)
{
  bool has_dirac_kernels_on_elem = _fe_problem.reinitDirac(elem, _tid);

  if(has_dirac_kernels_on_elem)
  {
    for(std::vector<DiracKernel *>::const_iterator dirac_kernel_it = _sys._dirac_kernels[_tid].all().begin();
        dirac_kernel_it != _sys._dirac_kernels[_tid].all().end();
        ++dirac_kernel_it)
    {
      DiracKernel * dirac = *dirac_kernel_it;

      if(dirac->hasPointsOnElem(elem))
      {
        if(_residual)
          dirac->computeResidual();
        else
        {
          dirac->subProblem().prepareShapes(dirac->variable().index(), _tid);
          dirac->computeJacobian();
        }
      }
    }
  }
}

void
ComputeDiracThread::postElement(const Elem * /*elem*/)
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  if (_residual)
    _fe_problem.addResidual(*_residual, _tid);
  else
    _fe_problem.addJacobian(*_jacobian, _tid);
}

void
ComputeDiracThread::join(const ComputeDiracThread & /*y*/)
{
}

