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
#include "InterfaceKernel.h"
#include "NonlocalKernel.h"
// libmesh includes
#include "libmesh/threads.h"

ComputeFullJacobianThread::ComputeFullJacobianThread(FEProblem & fe_problem, NonlinearSystem & sys, SparseMatrix<Number> & jacobian) :
    ComputeJacobianThread(fe_problem, sys, jacobian),
    _integrated_bcs(sys.getIntegratedBCWarehouse()),
    _dg_kernels(sys.getDGKernelWarehouse()),
    _interface_kernels(sys.getInterfaceKernelWarehouse()),
    _kernels(sys.getKernelWarehouse())
{
}

// Splitting Constructor
ComputeFullJacobianThread::ComputeFullJacobianThread(ComputeFullJacobianThread & x, Threads::split split) :
    ComputeJacobianThread(x, split),
    _integrated_bcs(x._integrated_bcs),
    _dg_kernels(x._dg_kernels),
    _interface_kernels(x._interface_kernels),
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
  for (const auto & it : ce)
  {
    MooseVariable & ivariable = *(it.first);
    MooseVariable & jvariable = *(it.second);

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    if (ivariable.activeOnSubdomain(_subdomain) && jvariable.activeOnSubdomain(_subdomain) && _kernels.hasActiveVariableBlockObjects(ivar, _subdomain, _tid))
    {
      // only if there are dofs for j-variable (if it is subdomain restricted var, there may not be any)
      const std::vector<MooseSharedPointer<KernelBase> > & kernels = _kernels.getActiveVariableBlockObjects(ivar, _subdomain, _tid);
      for (const auto & kernel : kernels)
        if ((kernel->variable().number() == ivar) && kernel->isImplicit())
        {
          kernel->subProblem().prepareShapes(jvar, _tid);
          kernel->computeOffDiagJacobian(jvar);
        }
    }
  }

  /// done only when nonlocal kernels exist in the system
  if (_fe_problem.checkNonlocalCouplingRequirement())
  {
    std::vector<std::pair<MooseVariable *, MooseVariable *> > & cne = _fe_problem.nonlocalCouplingEntries(_tid);
    for (const auto & it : cne)
    {
      MooseVariable & ivariable = *(it.first);
      MooseVariable & jvariable = *(it.second);

      unsigned int ivar = ivariable.number();
      unsigned int jvar = jvariable.number();

      if (ivariable.activeOnSubdomain(_subdomain) && jvariable.activeOnSubdomain(_subdomain) && _kernels.hasActiveVariableBlockObjects(ivar, _subdomain, _tid))
      {
        const std::vector<MooseSharedPointer<KernelBase> > & kernels = _kernels.getActiveVariableBlockObjects(ivar, _subdomain, _tid);
        for (const auto & kernel : kernels)
        {
          MooseSharedPointer<NonlocalKernel> nonlocal_kernel = MooseSharedNamespace::dynamic_pointer_cast<NonlocalKernel>(kernel);
          if (nonlocal_kernel)
            if ((kernel->variable().number() == ivar) && kernel->isImplicit())
            {
              kernel->subProblem().prepareShapes(jvar, _tid);
              kernel->computeNonlocalOffDiagJacobian(jvar);
            }
        }
      }
    }
  }

  const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);
  if (scalar_vars.size() > 0)
  {
    // go over nl-variables (non-scalar)
    const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
    for (const auto & ivariable : vars)
      if (ivariable->activeOnSubdomain(_subdomain) > 0 && _kernels.hasActiveVariableBlockObjects(ivariable->number(), _subdomain, _tid))
      {
        // for each variable get the list of active kernels
        const std::vector<MooseSharedPointer<KernelBase> > & kernels = _kernels.getActiveVariableBlockObjects(ivariable->number(), _subdomain, _tid);
        for (const auto & kernel : kernels)
          if (kernel->isImplicit())
          {
            // now, get the list of coupled scalar vars and compute their off-diag jacobians
            const std::vector<MooseVariableScalar *> coupled_scalar_vars = kernel->getCoupledMooseScalarVars();

            // Do: dvar / dscalar_var, only want to process only nl-variables (not aux ones)
            for (const auto & jvariable : coupled_scalar_vars)
              if (_sys.hasScalarVariable(jvariable->name()))
                kernel->computeOffDiagJacobianScalar(jvariable->number());
          }
      }
  }
}

void
ComputeFullJacobianThread::computeFaceJacobian(BoundaryID bnd_id)
{
  std::vector<std::pair<MooseVariable *, MooseVariable *> > & ce = _fe_problem.couplingEntries(_tid);
  for (const auto & it : ce)
  {
    MooseVariable & ivar = *(it.first);
    MooseVariable & jvar = *(it.second);
    if (ivar.activeOnSubdomain(_subdomain) && jvar.activeOnSubdomain(_subdomain) && _integrated_bcs.hasActiveBoundaryObjects(bnd_id, _tid))
    {
      // only if there are dofs for j-variable (if it is subdomain restricted var, there may not be any)
      const std::vector<MooseSharedPointer<IntegratedBC> > & bcs = _integrated_bcs.getBoundaryObjects(bnd_id, _tid);
      for (const auto & bc : bcs)
        if (bc->shouldApply() && bc->variable().number() == ivar.number() && bc->isImplicit())
        {
          bc->subProblem().prepareFaceShapes(jvar.number(), _tid);
          bc->computeJacobianBlock(jvar.number());
        }
    }
  }

  const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);
  if (scalar_vars.size() > 0)
  {
    // go over nl-variables (non-scalar)
    const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
    for (const auto & ivar : vars)
      if (ivar->activeOnSubdomain(_subdomain) > 0 && _integrated_bcs.hasActiveBoundaryObjects(bnd_id, _tid))
      {
        // for each variable get the list of active kernels
        const std::vector<MooseSharedPointer<IntegratedBC> > & bcs = _integrated_bcs.getActiveBoundaryObjects(bnd_id, _tid);
        for (const auto & bc : bcs)
          if (bc->variable().number() == ivar->number() && bc->isImplicit())
          {
            // now, get the list of coupled scalar vars and compute their off-diag jacobians
            const std::vector<MooseVariableScalar *> coupled_scalar_vars = bc->getCoupledMooseScalarVars();

            // Do: dvar / dscalar_var, only want to process only nl-variables (not aux ones)
            for (const auto & jvar : coupled_scalar_vars)
              if (_sys.hasScalarVariable(jvar->name()))
                bc->computeJacobianBlockScalar(jvar->number());
          }
      }
  }
}

void
ComputeFullJacobianThread::computeInternalFaceJacobian(const Elem * neighbor)
{
  if (_dg_kernels.hasActiveBlockObjects(_subdomain, _tid))
  {
    std::vector<std::pair<MooseVariable *, MooseVariable *> > & ce = _fe_problem.couplingEntries(_tid);
    for (const auto & it : ce)
    {
      const std::vector<MooseSharedPointer<DGKernel> > & dgks = _dg_kernels.getActiveBlockObjects(_subdomain, _tid);
      for (const auto & dg : dgks)
      {
        MooseVariable & ivariable = *(it.first);
        MooseVariable & jvariable = *(it.second);

        unsigned int ivar = ivariable.number();
        unsigned int jvar = jvariable.number();

        if (dg->variable().number() == ivar && dg->isImplicit() && dg->hasBlocks(neighbor->subdomain_id()) && jvariable.activeOnSubdomain(_subdomain))
        {
          dg->subProblem().prepareFaceShapes(jvar, _tid);
          dg->subProblem().prepareNeighborShapes(jvar, _tid);
          dg->computeOffDiagJacobian(jvar);
        }
      }
    }
  }
}

void
ComputeFullJacobianThread::computeInternalInterFaceJacobian(BoundaryID bnd_id)
{
  if (_interface_kernels.hasActiveBoundaryObjects(bnd_id, _tid))
  {
    std::vector<std::pair<MooseVariable *, MooseVariable *> > & ce = _fe_problem.couplingEntries(_tid);
    for (const auto & it : ce)
    {
      const std::vector<MooseSharedPointer<InterfaceKernel> > & int_ks = _interface_kernels.getActiveBoundaryObjects(bnd_id, _tid);
      for (const auto & interface_kernel : int_ks)
      {
        if (!interface_kernel->isImplicit())
          continue;

        unsigned int ivar = it.first->number();
        unsigned int jvar = it.second->number();

        interface_kernel->subProblem().prepareFaceShapes(jvar, _tid);
        interface_kernel->subProblem().prepareNeighborShapes(jvar, _tid);

        if (interface_kernel->variable().number() == ivar)
          interface_kernel->computeElementOffDiagJacobian(jvar);

        if (interface_kernel->neighborVariable().number() == ivar)
          interface_kernel->computeNeighborOffDiagJacobian(jvar);
      }
    }
  }
}
