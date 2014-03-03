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
  std::vector<std::pair<MooseVariable *, MooseVariable *> > & ce = _fe_problem.couplingEntries(_tid);
  for (std::vector<std::pair<MooseVariable *, MooseVariable *> >::iterator it = ce.begin(); it != ce.end(); ++it)
  {
    MooseVariable & ivariable = *(*it).first;
    MooseVariable & jvariable = *(*it).second;

    unsigned int ivar = ivariable.index();
    unsigned int jvar = jvariable.index();

    if (ivariable.activeOnSubdomain(_subdomain) && jvariable.activeOnSubdomain(_subdomain))
    {
      // only if there are dofs for j-variable (if it is subdomain restricted var, there may not be any)
      const std::vector<Kernel *> & kernels = _sys._kernels[_tid].activeVar(ivar);
      for (std::vector<Kernel *>::const_iterator kt = kernels.begin(); kt != kernels.end(); ++kt)
      {
        Kernel * kernel = *kt;
        if ((kernel->variable().index() == ivar) && kernel->isImplicit())
        {
          kernel->subProblem().prepareShapes(jvar, _tid);
          kernel->computeOffDiagJacobian(jvar);
        }
      }
    }
  }

  const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);
  if (scalar_vars.size() > 0)
  {
    // go over nl-variables (non-scalar)
    const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
    for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); it++)
    {
      MooseVariable & ivar = *(*it);
      if (ivar.activeOnSubdomain(_subdomain) > 0)
      {
        // for each variable get the list of active kernels
        const std::vector<Kernel *> & kernels = _sys._kernels[_tid].activeVar(ivar.index());
        for (std::vector<Kernel *>::const_iterator kt = kernels.begin(); kt != kernels.end(); ++kt)
        {
          Kernel * kernel = *kt;
          if (kernel->isImplicit())
          {
            // now, get the list of coupled scalar vars and compute their off-diag jacobians
            const std::vector<MooseVariableScalar *> coupled_scalar_vars = kernel->getCoupledMooseScalarVars();
            for (std::vector<MooseVariableScalar *>::const_iterator jt = coupled_scalar_vars.begin(); jt != coupled_scalar_vars.end(); jt++)
            {
              MooseVariableScalar & jvar = *(*jt);
              // Do: dvar / dscalar_var
              if (_sys.hasScalarVariable(jvar.name()))              // want to process only nl-variables (not aux ones)
                kernel->computeOffDiagJacobianScalar(jvar.index());
            }
          }
        }
      }
    }
  }
}

void
ComputeFullJacobianThread::computeFaceJacobian(BoundaryID bnd_id)
{
  std::vector<std::pair<MooseVariable *, MooseVariable *> > & ce = _fe_problem.couplingEntries(_tid);
  for (std::vector<std::pair<MooseVariable *, MooseVariable *> >::iterator it = ce.begin(); it != ce.end(); ++it)
  {
    MooseVariable & ivar = *(*it).first;
    MooseVariable & jvar = *(*it).second;
    if (ivar.activeOnSubdomain(_subdomain) && jvar.activeOnSubdomain(_subdomain))
    {
      // only if there are dofs for j-variable (if it is subdomain restricted var, there may not be any)
      std::vector<IntegratedBC *> bcs = _sys._bcs[_tid].getBCs(bnd_id);
      for (std::vector<IntegratedBC *>::iterator jt = bcs.begin(); jt != bcs.end(); ++jt)
      {
        IntegratedBC * bc = *jt;
        if (bc->shouldApply() && bc->variable().index() == ivar.index() && bc->isImplicit())
        {
          bc->subProblem().prepareFaceShapes(jvar.index(), _tid);
          bc->computeJacobianBlock(jvar.index());
        }
      }
    }
  }

  const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);
  if (scalar_vars.size() > 0)
  {
    // go over nl-variables (non-scalar)
    const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
    for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); it++)
    {
      MooseVariable & ivar = *(*it);
      if (ivar.activeOnSubdomain(_subdomain) > 0)
      {
        // for each variable get the list of active kernels
        std::vector<IntegratedBC *> bcs = _sys._bcs[_tid].activeIntegrated(bnd_id);
        for (std::vector<IntegratedBC *>::iterator kt = bcs.begin(); kt != bcs.end(); ++kt)
        {
          IntegratedBC * bc = *kt;
          if (bc->variable().index() == ivar.index() && bc->isImplicit())
          {
            // now, get the list of coupled scalar vars and compute their off-diag jacobians
            const std::vector<MooseVariableScalar *> coupled_scalar_vars = bc->getCoupledMooseScalarVars();
            for (std::vector<MooseVariableScalar *>::const_iterator jt = coupled_scalar_vars.begin(); jt != coupled_scalar_vars.end(); jt++)
            {
              MooseVariableScalar & jvar = *(*jt);
              // Do: dvar / dscalar_var
              if (_sys.hasScalarVariable(jvar.name()))              // want to process only nl-variables (not aux ones)
                bc->computeJacobianBlockScalar(jvar.index());
            }
          }
        }
      }
    }
  }
}

void
ComputeFullJacobianThread::computeInternalFaceJacobian()
{
  if (_sys._dg_kernels[_tid].active().empty())
    return;

  std::vector<std::pair<MooseVariable *, MooseVariable *> > & ce = _fe_problem.couplingEntries(_tid);
  for (std::vector<std::pair<MooseVariable *, MooseVariable *> >::iterator it = ce.begin(); it != ce.end(); ++it)
  {
    std::vector<DGKernel *> dgks = _sys._dg_kernels[_tid].active();
    for (std::vector<DGKernel *>::iterator dg_it = dgks.begin(); dg_it != dgks.end(); ++dg_it)
    {
      unsigned int ivar = (*it).first->index();
      DGKernel * dg = *dg_it;
      if (dg->variable().index() == ivar && dg->isImplicit())
      {
        unsigned int jvar = (*it).second->index();
        dg->subProblem().prepareNeighborShapes(jvar, _tid);
        dg->computeOffDiagJacobian(jvar);
      }
    }
  }
}

void
ComputeFullJacobianThread::postElement(const Elem * /*elem*/)
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  _fe_problem.addJacobian(_jacobian, _tid);
}
