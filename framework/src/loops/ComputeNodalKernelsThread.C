//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeNodalKernelsThread.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "NodalKernelBase.h"

#include "libmesh/threads.h"

ComputeNodalKernelsThread::ComputeNodalKernelsThread(
    FEProblemBase & fe_problem,
    MooseObjectTagWarehouse<NodalKernelBase> & nodal_kernels,
    const std::set<TagID> & tags)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(fe_problem),
    _fe_problem(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _tags(tags),
    _nodal_kernels(nodal_kernels),
    _num_cached(0)
{
}

// Splitting Constructor
ComputeNodalKernelsThread::ComputeNodalKernelsThread(ComputeNodalKernelsThread & x,
                                                     Threads::split split)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _fe_problem(x._fe_problem),
    _aux_sys(x._aux_sys),
    _tags(x._tags),
    _nodal_kernels(x._nodal_kernels),
    _num_cached(0)
{
}

void
ComputeNodalKernelsThread::pre()
{
  _num_cached = 0;

  if (!_tags.size() || _tags.size() == _fe_problem.numVectorTags(Moose::VECTOR_TAG_RESIDUAL))
    _nkernel_warehouse = &_nodal_kernels;
  else if (_tags.size() == 1)
    _nkernel_warehouse = &(_nodal_kernels.getVectorTagObjectWarehouse(*(_tags.begin()), _tid));
  else
    _nkernel_warehouse = &(_nodal_kernels.getVectorTagsObjectWarehouse(_tags, _tid));
}

void
ComputeNodalKernelsThread::onNode(ConstNodeRange::const_iterator & node_it)
{
  const Node * node = *node_it;

  std::set<const NodalKernelBase *> nks_executed;

  // prepare variables
  for (auto * var : _aux_sys._nodal_vars[_tid])
    var->prepareAux();

  _fe_problem.reinitNode(node, _tid);

  const std::set<SubdomainID> & block_ids = _aux_sys.mesh().getNodeBlockIds(*node);
  for (const auto & block : block_ids)
    if (_nkernel_warehouse->hasActiveBlockObjects(block, _tid))
    {
      std::set<TagID> needed_fe_var_vector_tags;
      _nkernel_warehouse->updateBlockFEVariableCoupledVectorTagDependency(
          block, needed_fe_var_vector_tags, _tid);
      _fe_problem.setActiveFEVariableCoupleableVectorTags(needed_fe_var_vector_tags, _tid);

      const auto & objects = _nkernel_warehouse->getActiveBlockObjects(block, _tid);
      for (const auto & nodal_kernel : objects)
        if (nks_executed.emplace(nodal_kernel.get()).second)
          nodal_kernel->computeResidual();
    }

  _num_cached++;

  if (_num_cached == 20) // Cache 20 nodes worth before adding into the residual
  {
    _num_cached = 0;
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _fe_problem.addCachedResidual(_tid);
  }
}

void
ComputeNodalKernelsThread::join(const ComputeNodalKernelsThread & /*y*/)
{
}

void
ComputeNodalKernelsThread::printGeneralExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid) || !_nkernel_warehouse->hasActiveObjects())
    return;

  const auto & console = _fe_problem.console();
  const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
  console << "[DBG] Computing nodal kernels contribution to residual on nodes on " << execute_on
          << std::endl;
  console << "[DBG] Nodes at boundaries between blocks will execute lower block ID first"
          << std::endl;
  console << "[DBG] Ordering of the nodal kernels on nodes they are defined on:" << std::endl;
  console << _nkernel_warehouse->activeObjectsToFormattedString() << std::endl;
}
