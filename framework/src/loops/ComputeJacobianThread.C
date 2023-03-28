//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeJacobianThread.h"

#include "DGKernelBase.h"
#include "FEProblem.h"
#include "IntegratedBCBase.h"
#include "InterfaceKernelBase.h"
#include "MooseVariableFE.h"
#include "NonlinearSystem.h"
#include "SwapBackSentinel.h"
#include "TimeDerivative.h"
#include "FVElementalKernel.h"
#include "MaterialBase.h"
#include "ConsoleUtils.h"

#include "libmesh/threads.h"

ComputeJacobianThread::ComputeJacobianThread(FEProblemBase & fe_problem,
                                             const std::set<TagID> & tags)
  : NonlinearThread(fe_problem), _tags(tags)
{
}

// Splitting Constructor
ComputeJacobianThread::ComputeJacobianThread(ComputeJacobianThread & x, Threads::split split)
  : NonlinearThread(x, split), _tags(x._tags)
{
}

ComputeJacobianThread::~ComputeJacobianThread() {}

void
ComputeJacobianThread::compute(KernelBase & kernel)
{
  if (kernel.isImplicit())
  {
    kernel.prepareShapes(kernel.variable().number());
    kernel.computeJacobian();
    if (_fe_problem.checkNonlocalCouplingRequirement() && !_fe_problem.computingScalingJacobian())
      mooseError("Nonlocal kernels only supported for non-diagonal coupling. Please specify an SMP "
                 "preconditioner, with appropriate row-column coupling or specify full = true.");
  }
}

void
ComputeJacobianThread::compute(FVElementalKernel & fvkernel)
{
  if (fvkernel.isImplicit())
    fvkernel.computeJacobian();
}

void
ComputeJacobianThread::compute(IntegratedBCBase & bc)
{
  if (bc.shouldApply() && bc.isImplicit())
  {
    bc.prepareShapes(bc.variable().number());
    bc.computeJacobian();
    if (_fe_problem.checkNonlocalCouplingRequirement() && !_fe_problem.computingScalingJacobian())
      mooseError("Nonlocal boundary conditions only supported for non-diagonal coupling. Please "
                 "specify an SMP preconditioner, with appropriate row-column coupling or specify "
                 "full = true.");
  }
}

void
ComputeJacobianThread::compute(DGKernelBase & dg, const Elem * neighbor)
{
  if (dg.isImplicit())
  {
    dg.prepareShapes(dg.variable().number());
    dg.prepareNeighborShapes(dg.variable().number());
    if (dg.hasBlocks(neighbor->subdomain_id()))
      dg.computeJacobian();
  }
}

void
ComputeJacobianThread::compute(InterfaceKernelBase & intk)
{
  if (intk.isImplicit())
  {
    intk.prepareShapes(intk.variable().number());
    intk.prepareNeighborShapes(intk.neighborVariable().number());
    intk.computeJacobian();
  }
}

void
ComputeJacobianThread::determineObjectWarehouses()
{
  // If users pass a empty vector or a full size of vector,
  // we take all kernels
  if (!_tags.size() || _tags.size() == _fe_problem.numMatrixTags())
  {
    _tag_kernels = &_kernels;
    _dg_warehouse = &_dg_kernels;
    _ibc_warehouse = &_integrated_bcs;
    _ik_warehouse = &_interface_kernels;
  }
  // If we have one tag only,
  // We call tag based storage
  else if (_tags.size() == 1)
  {
    _tag_kernels = &(_kernels.getMatrixTagObjectWarehouse(*(_tags.begin()), _tid));
    _dg_warehouse = &(_dg_kernels.getMatrixTagObjectWarehouse(*(_tags.begin()), _tid));
    _ibc_warehouse = &(_integrated_bcs.getMatrixTagObjectWarehouse(*(_tags.begin()), _tid));
    _ik_warehouse = &(_interface_kernels.getMatrixTagObjectWarehouse(*(_tags.begin()), _tid));
  }
  // This one may be expensive, and hopefully we do not use it so often
  else
  {
    _tag_kernels = &(_kernels.getMatrixTagsObjectWarehouse(_tags, _tid));
    _dg_warehouse = &(_dg_kernels.getMatrixTagsObjectWarehouse(_tags, _tid));
    _ibc_warehouse = &(_integrated_bcs.getMatrixTagsObjectWarehouse(_tags, _tid));
    _ik_warehouse = &(_interface_kernels.getMatrixTagsObjectWarehouse(_tags, _tid));
  }
}

void
ComputeJacobianThread::accumulateLower()
{
  _fe_problem.addJacobianLowerD(_tid);
}

void
ComputeJacobianThread::accumulateNeighborLower()
{
  _fe_problem.addJacobianNeighborLowerD(_tid);
}

void
ComputeJacobianThread::accumulateNeighbor()
{
  _fe_problem.addJacobianNeighbor(_tid);
}

void
ComputeJacobianThread::postElement(const Elem * /*elem*/)
{
  _fe_problem.cacheJacobian(_tid);
  _num_cached++;

  if (_num_cached % 20 == 0)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _fe_problem.addCachedJacobian(_tid);
  }
}

void
ComputeJacobianThread::join(const ComputeJacobianThread & /*y*/)
{
}
