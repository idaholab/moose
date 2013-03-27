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
#include "FEProblem.h"
#include "Kernel.h"
#include "IntegratedBC.h"
#include "DGKernel.h"
// libmesh includes
#include "libmesh/threads.h"

ComputeFullJacobianThread::ComputeFullJacobianThread(FEProblem & fe_problem, NonlinearSystem & sys, SparseMatrix<Number> & jacobian) :
    ComputeJacobianThread(fe_problem, sys, jacobian)
{
}

// Splitting Constructor
ComputeFullJacobianThread::ComputeFullJacobianThread(ComputeFullJacobianThread & x, Threads::split split) :
    ComputeJacobianThread(x, split)
{
}

ComputeFullJacobianThread::~ComputeFullJacobianThread()
{
}

void
ComputeFullJacobianThread::computeJacobian()
{
  std::vector<std::pair<unsigned int, unsigned int> > & ce = _fe_problem.couplingEntries(_tid);
  for (std::vector<std::pair<unsigned int, unsigned int> >::iterator it = ce.begin(); it != ce.end(); ++it)
  {
    unsigned int ivar = (*it).first;
    unsigned int jvar = (*it).second;

    MooseVariable & ivariable = _sys.getVariable(_tid, ivar);
    MooseVariable & jvariable = _sys.getVariable(_tid, jvar);

    if (ivariable.dofIndices().size() > 0 && jvariable.dofIndices().size() > 0)
    {
      // only if there are dofs for j-variable (if it is subdomain restricted var, there may not be any)
      const std::vector<Kernel *> & kernels = _sys._kernels[_tid].activeVar(ivar);
      for (std::vector<Kernel *>::const_iterator kt = kernels.begin(); kt != kernels.end(); ++kt)
      {
        Kernel * kernel = *kt;
        if (kernel->variable().index() == ivar)
        {
          kernel->subProblem().prepareShapes(jvar, _tid);
          kernel->computeOffDiagJacobian(jvar);
        }
      }
    }
  }

  const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);
  for (std::vector<MooseVariableScalar *>::const_iterator jt = scalar_vars.begin(); jt != scalar_vars.end(); jt++)
  {
    // Do: dvar / dscalar_var
    MooseVariableScalar & jvar = *(*jt);

    const std::vector<Kernel *> & kernels = _sys._kernels[_tid].active();
    for (std::vector<Kernel *>::const_iterator kt = kernels.begin(); kt != kernels.end(); ++kt)
    {
      Kernel * kernel = *kt;
      MooseVariable & ivar = kernel->variable();
      if (ivar.dofIndices().size() > 0)
      {
        kernel->subProblem().prepareShapes(ivar.index(), _tid);
        kernel->computeOffDiagJacobianScalar(jvar.index());
      }
    }
  }
}

void
ComputeFullJacobianThread::computeFaceJacobian(BoundaryID bnd_id)
{
  std::vector<std::pair<unsigned int, unsigned int> > & ce = _fe_problem.couplingEntries(_tid);
  for (std::vector<std::pair<unsigned int, unsigned int> >::iterator it = ce.begin(); it != ce.end(); ++it)
  {
    unsigned int ivar = (*it).first;
    unsigned int jvar = (*it).second;

    MooseVariable & var = _sys.getVariable(_tid, jvar);
    if (var.dofIndices().size() > 0)
    {
      // only if there are dofs for j-variable (if it is subdomain restricted var, there may not be any)
      std::vector<IntegratedBC *> bcs = _sys._bcs[_tid].getBCs(bnd_id);
      for (std::vector<IntegratedBC *>::iterator jt = bcs.begin(); jt != bcs.end(); ++jt)
      {
        IntegratedBC * bc = *jt;
        if (bc->shouldApply() && bc->variable().index() == ivar)
        {
          bc->subProblem().prepareFaceShapes(jvar, _tid);
          bc->computeJacobianBlock(jvar);
        }
      }
    }
  }

  const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);
  for (std::vector<MooseVariableScalar *>::const_iterator jt = scalar_vars.begin(); jt != scalar_vars.end(); jt++)
  {
    // Do: dvar / dscalar_var
    MooseVariableScalar & jvar = *(*jt);

    std::vector<IntegratedBC *> bcs = _sys._bcs[_tid].getBCs(bnd_id);
    for (std::vector<IntegratedBC *>::iterator kt = bcs.begin(); kt != bcs.end(); ++kt)
    {
      IntegratedBC * bc = *kt;
      if (bc->shouldApply())
      {
        MooseVariable & ivar = bc->variable();
        if (ivar.dofIndices().size() > 0)
        {
          bc->subProblem().prepareFaceShapes(ivar.index(), _tid);
          bc->computeJacobianBlockScalar(jvar.index());
        }
      }
    }
  }
}

void
ComputeFullJacobianThread::computeInternalFaceJacobian()
{
  std::vector<std::pair<unsigned int, unsigned int> > & ce = _fe_problem.couplingEntries(_tid);
  for (std::vector<std::pair<unsigned int, unsigned int> >::iterator it = ce.begin(); it != ce.end(); ++it)
  {
    unsigned int ivar = (*it).first;
    unsigned int jvar = (*it).second;

    std::vector<DGKernel *> dgks = _sys._dg_kernels[_tid].active();
    for (std::vector<DGKernel *>::iterator it = dgks.begin(); it != dgks.end(); ++it)
    {
      DGKernel * dg = *it;
      if (dg->variable().index() == ivar)
      {
        dg->subProblem().prepareNeighborShapes(jvar, _tid);
        dg->computeOffDiagJacobian(jvar);
      }
    }
  }
}
