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

#include "ComputeFullJacobianThread.h"
#include "NonlinearSystem.h"
#include "Problem.h"

// libmesh includes
#include "threads.h"

ComputeFullJacobianThread::ComputeFullJacobianThread(Problem & problem, NonlinearSystem & sys, SparseMatrix<Number> & jacobian) :
    ComputeJacobianThread(problem, sys, jacobian)
{
}

// Splitting Constructor
ComputeFullJacobianThread::ComputeFullJacobianThread(ComputeFullJacobianThread & x, Threads::split split) :
    ComputeJacobianThread(x, split)
{
}

void
ComputeFullJacobianThread::computeJacobian()
{
  std::vector<std::pair<unsigned int, unsigned int> > & ce = _sys.couplingEntries(_tid);
  for (std::vector<std::pair<unsigned int, unsigned int> >::iterator it = ce.begin(); it != ce.end(); ++it)
  {
    unsigned int ivar = (*it).first;
    unsigned int jvar = (*it).second;

    const std::vector<Kernel *> & kernels = _sys._kernels[_tid].activeVar(ivar);
    for (std::vector<Kernel *>::const_iterator kt = kernels.begin(); kt != kernels.end(); ++kt)
    {
      Kernel * kernel = *kt;
      if (kernel->variable().number() == ivar)
      {
        kernel->subProblem().prepareShapes(jvar, _tid);
        kernel->computeOffDiagJacobian(jvar);
      }
    }
  }
}

void
ComputeFullJacobianThread::computeFaceJacobian(short int bnd_id)
{
  std::vector<std::pair<unsigned int, unsigned int> > & ce = _sys.couplingEntries(_tid);
  for (std::vector<std::pair<unsigned int, unsigned int> >::iterator it = ce.begin(); it != ce.end(); ++it)
  {
    unsigned int ivar = (*it).first;
    unsigned int jvar = (*it).second;

    std::vector<IntegratedBC *> bcs = _sys._bcs[_tid].getBCs(bnd_id);
    for (std::vector<IntegratedBC *>::iterator jt = bcs.begin(); jt != bcs.end(); ++jt)
    {
      IntegratedBC * bc = *jt;
      if (bc->variable().number() == ivar)
      {
        bc->subProblem().prepareFaceShapes(jvar, _tid);
        bc->computeJacobianBlock(jvar);
      }
    }
  }
}
