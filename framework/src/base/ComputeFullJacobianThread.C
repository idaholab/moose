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
    ComputeJacobianThread(fe_problem, sys, jacobian),
    _integrated_bcs(sys.getIntegratedBCWarehouse()),
    _dg_kernels(sys.getDGKernelWarehouse()),
    _kernels(sys.getKernelWarehouse())
{
}

// Splitting Constructor
ComputeFullJacobianThread::ComputeFullJacobianThread(ComputeFullJacobianThread & x, Threads::split split) :
    ComputeJacobianThread(x, split),
    _integrated_bcs(x._integrated_bcs),
    _dg_kernels(x._dg_kernels),
    _kernels(x._kernels)
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

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    if (ivariable.activeOnSubdomain(_subdomain) && jvariable.activeOnSubdomain(_subdomain) && _kernels.hasActiveVariableBlockObjects(ivar, _subdomain, _tid))
    {
      // only if there are dofs for j-variable (if it is subdomain restricted var, there may not be any)
      const std::vector<MooseSharedPointer<KernelBase> > & kernels = _kernels.getActiveVariableBlockObjects(ivar, _subdomain, _tid);
      for (std::vector<MooseSharedPointer<KernelBase> >::const_iterator kt = kernels.begin(); kt != kernels.end(); ++kt)
      {
        MooseSharedPointer<KernelBase> kernel = *kt;
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
      MooseVariable & ivariable = *(*it);
      if (ivariable.activeOnSubdomain(_subdomain) > 0 && _kernels.hasActiveVariableBlockObjects(ivariable.number(), _subdomain, _tid))
      {
        // for each variable get the list of active kernels
        const std::vector<MooseSharedPointer<KernelBase> > & kernels = _kernels.getActiveVariableBlockObjects(ivariable.number(), _subdomain, _tid);
        for (std::vector<MooseSharedPointer<KernelBase> >::const_iterator kt = kernels.begin(); kt != kernels.end(); ++kt)
        {
          MooseSharedPointer<KernelBase> kernel = *kt;
          if (kernel->isImplicit())
          {
            // now, get the list of coupled scalar vars and compute their off-diag jacobians
            const std::vector<MooseVariableScalar *> coupled_scalar_vars = kernel->getCoupledMooseScalarVars();
            for (std::vector<MooseVariableScalar *>::const_iterator jt = coupled_scalar_vars.begin(); jt != coupled_scalar_vars.end(); jt++)
            {
              MooseVariableScalar & jvariable = *(*jt);
              // Do: dvar / dscalar_var
              if (_sys.hasScalarVariable(jvariable.name()))              // want to process only nl-variables (not aux ones)
                kernel->computeOffDiagJacobianScalar(jvariable.number());
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
    if (ivar.activeOnSubdomain(_subdomain) && jvar.activeOnSubdomain(_subdomain) && _integrated_bcs.hasActiveBoundaryObjects(bnd_id, _tid))
    {
      // only if there are dofs for j-variable (if it is subdomain restricted var, there may not be any)

      const std::vector<MooseSharedPointer<IntegratedBC> > & bcs = _integrated_bcs.getBoundaryObjects(bnd_id, _tid);
      for (std::vector<MooseSharedPointer<IntegratedBC> >::const_iterator jt = bcs.begin(); jt != bcs.end(); ++jt)
      {
        MooseSharedPointer<IntegratedBC> bc = *jt;
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
      if (ivar.activeOnSubdomain(_subdomain) > 0 && _integrated_bcs.hasActiveBoundaryObjects(bnd_id, _tid))
      {
        // for each variable get the list of active kernels
        const std::vector<MooseSharedPointer<IntegratedBC> > & bcs = _integrated_bcs.getActiveBoundaryObjects(bnd_id, _tid);
        for (std::vector<MooseSharedPointer<IntegratedBC> >::const_iterator kt = bcs.begin(); kt != bcs.end(); ++kt)
        {
          MooseSharedPointer<IntegratedBC> bc = *kt;
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
  if (_dg_kernels.hasActiveObjects(_tid))
  {
    std::vector<std::pair<MooseVariable *, MooseVariable *> > & ce = _fe_problem.couplingEntries(_tid);
    for (std::vector<std::pair<MooseVariable *, MooseVariable *> >::iterator it = ce.begin(); it != ce.end(); ++it)
    {
      const std::vector<MooseSharedPointer<DGKernel> > & dgks = _dg_kernels.getActiveObjects(_tid);
      for (std::vector<MooseSharedPointer<DGKernel> >::const_iterator dg_it = dgks.begin(); dg_it != dgks.end(); ++dg_it)
      {
        unsigned int ivar = (*it).first->number();
        MooseSharedPointer<DGKernel> dg = *dg_it;
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
}
