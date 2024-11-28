//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeFullJacobianThread.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "KernelBase.h"
#include "IntegratedBCBase.h"
#include "DGKernel.h"
#include "InterfaceKernelBase.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "NonlocalKernel.h"
#include "NonlocalIntegratedBC.h"
#include "FVElementalKernel.h"
#include "libmesh/threads.h"

ComputeFullJacobianThread::ComputeFullJacobianThread(FEProblemBase & fe_problem,
                                                     const std::set<TagID> & tags)
  : ComputeJacobianThread(fe_problem, tags)
{
}

// Splitting Constructor
ComputeFullJacobianThread::ComputeFullJacobianThread(ComputeFullJacobianThread & x,
                                                     Threads::split split)
  : ComputeJacobianThread(x, split)
{
}

ComputeFullJacobianThread::~ComputeFullJacobianThread() {}

void
ComputeFullJacobianThread::computeOnElement()
{
  auto & ce = _fe_problem.couplingEntries(_tid, _nl.number());
  for (const auto & it : ce)
  {
    MooseVariableFieldBase & ivariable = *(it.first);
    MooseVariableFieldBase & jvariable = *(it.second);

    if (ivariable.isFV())
      continue;

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    if (ivariable.activeOnSubdomain(_subdomain) && jvariable.activeOnSubdomain(_subdomain) &&
        _tag_kernels->hasActiveVariableBlockObjects(ivar, _subdomain, _tid))
    {
      // only if there are dofs for j-variable (if it is subdomain restricted var, there may not be
      // any)
      const auto & kernels = _tag_kernels->getActiveVariableBlockObjects(ivar, _subdomain, _tid);
      for (const auto & kernel : kernels)
        if ((kernel->variable().number() == ivar) && kernel->isImplicit())
        {
          kernel->prepareShapes(jvar);
          kernel->computeOffDiagJacobian(jvar);
        }
    }
  }

  /// done only when nonlocal kernels exist in the system
  if (_fe_problem.checkNonlocalCouplingRequirement())
  {
    auto & cne = _fe_problem.nonlocalCouplingEntries(_tid, _nl.number());
    for (const auto & it : cne)
    {
      MooseVariableFieldBase & ivariable = *(it.first);
      MooseVariableFieldBase & jvariable = *(it.second);

      if (ivariable.isFV())
        continue;

      unsigned int ivar = ivariable.number();
      unsigned int jvar = jvariable.number();

      if (ivariable.activeOnSubdomain(_subdomain) && jvariable.activeOnSubdomain(_subdomain) &&
          _tag_kernels->hasActiveVariableBlockObjects(ivar, _subdomain, _tid))
      {
        const auto & kernels = _tag_kernels->getActiveVariableBlockObjects(ivar, _subdomain, _tid);
        for (const auto & kernel : kernels)
        {
          std::shared_ptr<NonlocalKernel> nonlocal_kernel =
              std::dynamic_pointer_cast<NonlocalKernel>(kernel);
          if (nonlocal_kernel)
            if ((kernel->variable().number() == ivar) && kernel->isImplicit())
            {
              kernel->prepareShapes(jvar);
              kernel->computeNonlocalOffDiagJacobian(jvar);
            }
        }
      }
    }
  }

  const std::vector<MooseVariableScalar *> & scalar_vars = _nl.getScalarVariables(_tid);
  if (scalar_vars.size() > 0)
  {
    // go over nl-variables (non-scalar)
    const std::vector<MooseVariableFieldBase *> & vars = _nl.getVariables(_tid);
    for (const auto & ivariable : vars)
      if (ivariable->activeOnSubdomain(_subdomain) > 0 &&
          _tag_kernels->hasActiveVariableBlockObjects(ivariable->number(), _subdomain, _tid))
      {
        // for each variable get the list of active kernels
        const auto & kernels =
            _tag_kernels->getActiveVariableBlockObjects(ivariable->number(), _subdomain, _tid);
        for (const auto & kernel : kernels)
          if (kernel->isImplicit())
          {
            // now, get the list of coupled scalar vars and compute their off-diag jacobians
            const auto & coupled_scalar_vars = kernel->getCoupledMooseScalarVars();

            // Do: dvar / dscalar_var, only want to process only nl-variables (not aux ones)
            for (const auto & jvariable : coupled_scalar_vars)
              if (_nl.hasScalarVariable(jvariable->name()))
                kernel->computeOffDiagJacobianScalar(jvariable->number());
          }
      }
  }

  if (_fe_problem.haveFV())
    for (auto fv_kernel : _fv_kernels)
      if (fv_kernel->isImplicit())
        fv_kernel->computeOffDiagJacobian();
}

void
ComputeFullJacobianThread::computeOnBoundary(BoundaryID bnd_id, const Elem * lower_d_elem)
{
  auto & ce = _fe_problem.couplingEntries(_tid, _nl.number());
  for (const auto & it : ce)
  {
    MooseVariableFieldBase & ivariable = *(it.first);
    MooseVariableFieldBase & jvariable = *(it.second);

    // We don't currently support coupling with FV variables
    if (ivariable.isFV() || jvariable.isFV())
      continue;

    const auto ivar = ivariable.number();
    const auto jvar = jvariable.number();

    if (!ivariable.activeOnSubdomain(_subdomain))
      continue;

    // only if there are dofs for j-variable (if it is subdomain restricted var, there may not be
    // any)
    if (lower_d_elem)
    {
      auto lower_d_subdomain = lower_d_elem->subdomain_id();
      if (!jvariable.activeOnSubdomain(_subdomain) &&
          !jvariable.activeOnSubdomain(lower_d_subdomain))
        continue;
    }
    else
    {
      if (!jvariable.activeOnSubdomain(_subdomain))
        continue;
    }

    if (!_ibc_warehouse->hasActiveBoundaryObjects(bnd_id, _tid))
      continue;

    const auto & bcs = _ibc_warehouse->getActiveBoundaryObjects(bnd_id, _tid);
    for (const auto & bc : bcs)
      if (bc->shouldApply() && bc->variable().number() == ivar && bc->isImplicit())
      {
        bc->prepareShapes(jvar);
        bc->computeOffDiagJacobian(jvar);
      }
  }

  /// done only when nonlocal integrated_bcs exist in the system
  if (_fe_problem.checkNonlocalCouplingRequirement())
  {
    auto & cne = _fe_problem.nonlocalCouplingEntries(_tid, _nl.number());
    for (const auto & it : cne)
    {
      MooseVariableFieldBase & ivariable = *(it.first);
      MooseVariableFieldBase & jvariable = *(it.second);

      if (ivariable.isFV())
        continue;

      unsigned int ivar = ivariable.number();
      unsigned int jvar = jvariable.number();

      if (ivariable.activeOnSubdomain(_subdomain) && jvariable.activeOnSubdomain(_subdomain) &&
          _ibc_warehouse->hasActiveBoundaryObjects(bnd_id, _tid))
      {
        const std::vector<std::shared_ptr<IntegratedBCBase>> & integrated_bcs =
            _ibc_warehouse->getBoundaryObjects(bnd_id, _tid);
        for (const auto & integrated_bc : integrated_bcs)
        {
          std::shared_ptr<NonlocalIntegratedBC> nonlocal_integrated_bc =
              std::dynamic_pointer_cast<NonlocalIntegratedBC>(integrated_bc);
          if (nonlocal_integrated_bc)
            if ((integrated_bc->variable().number() == ivar) && integrated_bc->isImplicit())
            {
              integrated_bc->prepareShapes(jvar);
              integrated_bc->computeNonlocalOffDiagJacobian(jvar);
            }
        }
      }
    }
  }

  const std::vector<MooseVariableScalar *> & scalar_vars = _nl.getScalarVariables(_tid);
  if (scalar_vars.size() > 0)
  {
    // go over nl-variables (non-scalar)
    const std::vector<MooseVariableFieldBase *> & vars = _nl.getVariables(_tid);
    for (const auto & ivar : vars)
      if (ivar->activeOnSubdomain(_subdomain) > 0 &&
          _ibc_warehouse->hasActiveBoundaryObjects(bnd_id, _tid))
      {
        // for each variable get the list of active kernels
        const auto & bcs = _ibc_warehouse->getActiveBoundaryObjects(bnd_id, _tid);
        for (const auto & bc : bcs)
          if (bc->variable().number() == ivar->number() && bc->isImplicit())
          {
            // now, get the list of coupled scalar vars and compute their off-diag jacobians
            const std::vector<MooseVariableScalar *> coupled_scalar_vars =
                bc->getCoupledMooseScalarVars();

            // Do: dvar / dscalar_var, only want to process only nl-variables (not aux ones)
            for (const auto & jvar : coupled_scalar_vars)
              if (_nl.hasScalarVariable(jvar->name()))
                bc->computeOffDiagJacobianScalar(jvar->number());
          }
      }
  }
}

void
ComputeFullJacobianThread::computeOnInterface(BoundaryID bnd_id)
{
  if (_ik_warehouse->hasActiveBoundaryObjects(bnd_id, _tid))
  {
    const auto & ce = _fe_problem.couplingEntries(_tid, _nl.number());
    for (const auto & it : ce)
    {
      MooseVariableFieldBase & ivariable = *(it.first);
      MooseVariableFieldBase & jvariable = *(it.second);

      if (ivariable.isFV())
        continue;

      unsigned int ivar = ivariable.number();
      unsigned int jvar = jvariable.number();

      const auto & int_ks = _ik_warehouse->getActiveBoundaryObjects(bnd_id, _tid);
      for (const auto & interface_kernel : int_ks)
      {
        if (!interface_kernel->isImplicit())
          continue;

        interface_kernel->prepareShapes(jvar);
        interface_kernel->prepareNeighborShapes(jvar);

        if (interface_kernel->variable().number() == ivar)
          interface_kernel->computeElementOffDiagJacobian(jvar);

        if (interface_kernel->neighborVariable().number() == ivar)
          interface_kernel->computeNeighborOffDiagJacobian(jvar);
      }
    }
  }
}

void
ComputeFullJacobianThread::computeOnInternalFace(const Elem * neighbor)
{
  if (_dg_warehouse->hasActiveBlockObjects(_subdomain, _tid))
  {
    const auto & ce = _fe_problem.couplingEntries(_tid, _nl.number());
    for (const auto & it : ce)
    {
      MooseVariableFieldBase & ivariable = *(it.first);
      MooseVariableFieldBase & jvariable = *(it.second);

      if (ivariable.isFV())
        continue;

      unsigned int ivar = ivariable.number();
      unsigned int jvar = jvariable.number();

      const auto & dgks = _dg_warehouse->getActiveBlockObjects(_subdomain, _tid);
      for (const auto & dg : dgks)
      {
        // this check may skip some couplings...
        if (dg->variable().number() == ivar && dg->isImplicit() &&
            dg->hasBlocks(neighbor->subdomain_id()) &&
            (jvariable.activeOnSubdomain(_subdomain) ||
             jvariable.activeOnSubdomains(_fe_problem.mesh().interiorLowerDBlocks())))
        {
          dg->prepareShapes(jvar);
          dg->prepareNeighborShapes(jvar);
          dg->computeOffDiagJacobian(jvar);
        }
      }
    }
  }
}
