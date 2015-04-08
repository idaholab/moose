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
#include "KernelBase.h"
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
  const KernelWarehouse & kernel_warehouse = _sys.getKernelWarehouse(_tid);

  std::vector<std::pair<MooseVariable *, MooseVariable *> > & ce = _fe_problem.couplingEntries(_tid);
  for (std::vector<std::pair<MooseVariable *, MooseVariable *> >::iterator it = ce.begin(); it != ce.end(); ++it)
  {
    MooseVariable & ivariable = *(*it).first;
    MooseVariable & jvariable = *(*it).second;

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    if (ivariable.activeOnSubdomain(_subdomain) && jvariable.activeOnSubdomain(_subdomain)
        && kernel_warehouse.hasActiveKernels(ivar))
    {
      // only if there are dofs for j-variable (if it is subdomain restricted var, there may not be any)
      const std::vector<KernelBase *> & kernels = kernel_warehouse.activeVar(ivar);
      for (std::vector<KernelBase *>::const_iterator kt = kernels.begin(); kt != kernels.end(); ++kt)
      {
        KernelBase * kernel = *kt;
        if ((kernel->variable().number() == ivar) && kernel->isImplicit())
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
      if (ivar.activeOnSubdomain(_subdomain) > 0 && kernel_warehouse.hasActiveKernels(ivar.number()))
      {
        // for each variable get the list of active kernels
        const std::vector<KernelBase *> & kernels = kernel_warehouse.activeVar(ivar.number());
        for (std::vector<KernelBase *>::const_iterator kt = kernels.begin(); kt != kernels.end(); ++kt)
        {
          KernelBase * kernel = *kt;
          if (kernel->isImplicit())
          {
            // now, get the list of coupled scalar vars and compute their off-diag jacobians
            const std::vector<MooseVariableScalar *> coupled_scalar_vars = kernel->getCoupledMooseScalarVars();
            for (std::vector<MooseVariableScalar *>::const_iterator jt = coupled_scalar_vars.begin(); jt != coupled_scalar_vars.end(); jt++)
            {
              MooseVariableScalar & jvar = *(*jt);
              // Do: dvar / dscalar_var
              if (_sys.hasScalarVariable(jvar.name()))              // want to process only nl-variables (not aux ones)
                kernel->computeOffDiagJacobianScalar(jvar.number());
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
      std::vector<IntegratedBC *> bcs = _sys.getBCWarehouse(_tid).getBCs(bnd_id);
      for (std::vector<IntegratedBC *>::iterator jt = bcs.begin(); jt != bcs.end(); ++jt)
      {
        IntegratedBC * bc = *jt;
        if (bc->shouldApply() && bc->variable().number() == ivar.number() && bc->isImplicit())
        {
          bc->subProblem().prepareFaceShapes(jvar.number(), _tid);
          bc->computeJacobianBlock(jvar.number());
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
        std::vector<IntegratedBC *> bcs;
        _sys.getBCWarehouse(_tid).activeIntegrated(bnd_id, bcs);
        for (std::vector<IntegratedBC *>::iterator kt = bcs.begin(); kt != bcs.end(); ++kt)
        {
          IntegratedBC * bc = *kt;
          if (bc->variable().number() == ivar.number() && bc->isImplicit())
          {
            // now, get the list of coupled scalar vars and compute their off-diag jacobians
            const std::vector<MooseVariableScalar *> coupled_scalar_vars = bc->getCoupledMooseScalarVars();
            for (std::vector<MooseVariableScalar *>::const_iterator jt = coupled_scalar_vars.begin(); jt != coupled_scalar_vars.end(); jt++)
            {
              MooseVariableScalar & jvar = *(*jt);
              // Do: dvar / dscalar_var
              if (_sys.hasScalarVariable(jvar.name()))              // want to process only nl-variables (not aux ones)
                bc->computeJacobianBlockScalar(jvar.number());
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
  std::vector<std::pair<MooseVariable *, MooseVariable *> > & ce = _fe_problem.couplingEntries(_tid);
  for (std::vector<std::pair<MooseVariable *, MooseVariable *> >::iterator it = ce.begin(); it != ce.end(); ++it)
  {
    std::vector<DGKernel *> dgks = _sys.getDGKernelWarehouse(_tid).active();
    for (std::vector<DGKernel *>::iterator dg_it = dgks.begin(); dg_it != dgks.end(); ++dg_it)
    {
      unsigned int ivar = (*it).first->number();
      DGKernel * dg = *dg_it;
      if (dg->variable().number() == ivar && dg->isImplicit())
      {
        unsigned int jvar = (*it).second->number();
        dg->subProblem().prepareFaceShapes(dg->variable().number(), _tid);
        dg->subProblem().prepareNeighborShapes(jvar, _tid);
        dg->computeOffDiagJacobian(jvar);
      }
    }
  }
}
