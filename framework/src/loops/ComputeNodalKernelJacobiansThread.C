//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeNodalKernelJacobiansThread.h"

// MOOSE includes
#include "Assembly.h"
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "NodalKernelBase.h"
#include "NonlinearSystemBase.h"

#include "libmesh/sparse_matrix.h"

ComputeNodalKernelJacobiansThread::ComputeNodalKernelJacobiansThread(
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
ComputeNodalKernelJacobiansThread::ComputeNodalKernelJacobiansThread(
    ComputeNodalKernelJacobiansThread & x, Threads::split split)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _fe_problem(x._fe_problem),
    _aux_sys(x._aux_sys),
    _tags(x._tags),
    _nodal_kernels(x._nodal_kernels),
    _num_cached(0)
{
}

void
ComputeNodalKernelJacobiansThread::pre()
{
  _num_cached = 0;

  if (!_tags.size() || _tags.size() == _fe_problem.numMatrixTags())
    _nkernel_warehouse = &_nodal_kernels;
  else if (_tags.size() == 1)
    _nkernel_warehouse = &(_nodal_kernels.getMatrixTagObjectWarehouse(*(_tags.begin()), _tid));
  else
    _nkernel_warehouse = &(_nodal_kernels.getMatrixTagsObjectWarehouse(_tags, _tid));
}

void
ComputeNodalKernelJacobiansThread::onNode(ConstNodeRange::const_iterator & node_it)
{
  const Node * node = *node_it;

  auto & ce = _fe_problem.couplingEntries(_tid);
  for (const auto & it : ce)
  {
    MooseVariableFEBase & ivariable = *(it.first);
    MooseVariableFEBase & jvariable = *(it.second);

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    // The NodalKernels that are active and are coupled to the jvar in question
    std::vector<std::shared_ptr<NodalKernelBase>> active_involved_kernels;

    const std::set<SubdomainID> & block_ids = _aux_sys.mesh().getNodeBlockIds(*node);
    for (const auto & block : block_ids)
    {
      if (_nkernel_warehouse->hasActiveBlockObjects(block, _tid))
      {
        // Loop over each NodalKernel to see if it's involved with the jvar
        const auto & objects = _nkernel_warehouse->getActiveBlockObjects(block, _tid);
        for (const auto & nodal_kernel : objects)
        {
          if (nodal_kernel->variable().number() == ivar)
          {
            // If this NodalKernel is acting on the jvar add it to the list and short-circuit the
            // loop
            if (nodal_kernel->variable().number() == jvar)
            {
              active_involved_kernels.push_back(nodal_kernel);
              continue;
            }

            // See if this NodalKernel is coupled to the jvar
            const std::vector<MooseVariableFEBase *> & coupled_vars =
                nodal_kernel->getCoupledMooseVars();
            for (const auto & var : coupled_vars)
              if (var->number() == jvar)
              {
                active_involved_kernels.push_back(nodal_kernel);
                break; // It only takes one
              }
          }
        }
      }
    }

    // Did we find any NodalKernels coupled to this jvar?
    if (!active_involved_kernels.empty())
    {
      // prepare variables
      for (auto * var : _aux_sys._nodal_vars[_tid])
        var->prepareAux();

      _fe_problem.reinitNode(node, _tid);

      for (const auto & nodal_kernel : active_involved_kernels)
        nodal_kernel->computeOffDiagJacobian(jvar);

      _num_cached++;

      if (_num_cached == 20) // Cache 20 nodes worth before adding into the residual
      {
        _num_cached = 0;
        // vectors are thread-safe, but matrices are not yet
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.assembly(_tid, _fe_problem.currentNonlinearSystem().number())
            .addCachedJacobian();
      }
    }
  }
}

void
ComputeNodalKernelJacobiansThread::join(const ComputeNodalKernelJacobiansThread & /*y*/)
{
}

void
ComputeNodalKernelJacobiansThread::printGeneralExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid) || !_nkernel_warehouse->hasActiveObjects())
    return;

  const auto & console = _fe_problem.console();
  const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
  console << "[DBG] Executing nodal kernels contribution to Jacobian on nodes on " << execute_on
          << std::endl;
  console << _nkernel_warehouse->activeObjectsToFormattedString() << std::endl;
}
