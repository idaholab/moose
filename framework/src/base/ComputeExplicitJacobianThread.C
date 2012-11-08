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

#include "ComputeExplicitJacobianThread.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "TimeDerivative.h"

// libmesh includes
#include "threads.h"

ComputeExplicitJacobianThread::ComputeExplicitJacobianThread(FEProblem & fe_problem, NonlinearSystem & sys, SparseMatrix<Number> & jacobian) :
    ComputeJacobianThread(fe_problem, sys, jacobian)
{
}

// Splitting Constructor
ComputeExplicitJacobianThread::ComputeExplicitJacobianThread(ComputeExplicitJacobianThread & x, Threads::split split) :
    ComputeJacobianThread(x, split)
{
}

ComputeExplicitJacobianThread::~ComputeExplicitJacobianThread()
{
}

void
ComputeExplicitJacobianThread::computeJacobian()
{
  // process only time derivative kernel
  const std::vector<Kernel *> & kernels = _sys._kernels[_tid].activeTime();
  for (std::vector<Kernel *>::const_iterator it = kernels.begin(); it != kernels.end(); ++it)
  {
    Kernel * kernel = *it;
     kernel->subProblem().prepareShapes(kernel->variable().number(), _tid);
     kernel->computeJacobian();
  }
}
