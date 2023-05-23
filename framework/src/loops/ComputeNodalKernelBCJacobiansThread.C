//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeNodalKernelBCJacobiansThread.h"

// MOOSE includes
#include "Assembly.h"
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "MooseVariableFE.h"
#include "NodalKernelBase.h"
#include "NonlinearSystemBase.h"

#include "libmesh/threads.h"

ComputeNodalKernelBCJacobiansThread::ComputeNodalKernelBCJacobiansThread(
    FEProblemBase & fe_problem,
    MooseObjectTagWarehouse<NodalKernelBase> & nodal_kernels,
    const std::set<TagID> & tags)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _fe_problem(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _tags(tags),
    _nodal_kernels(nodal_kernels),
    _num_cached(0)
{
}

// Splitting Constructor
ComputeNodalKernelBCJacobiansThread::ComputeNodalKernelBCJacobiansThread(
    ComputeNodalKernelBCJacobiansThread & x, Threads::split split)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split),
    _fe_problem(x._fe_problem),
    _aux_sys(x._aux_sys),
    _tags(x._tags),
    _nodal_kernels(x._nodal_kernels),
    _num_cached(0)
{
}

void
ComputeNodalKernelBCJacobiansThread::pre()
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
ComputeNodalKernelBCJacobiansThread::onNode(ConstBndNodeRange::const_iterator & node_it)
{
  const BndNode * bnode = *node_it;

  BoundaryID boundary_id = bnode->_bnd_id;

  auto & ce = _fe_problem.couplingEntries(_tid);
  for (const auto & it : ce)
  {
    MooseVariableFEBase & ivariable = *(it.first);
    MooseVariableFEBase & jvariable = *(it.second);

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    // The NodalKernels that are active and are coupled to the jvar in question
    std::vector<std::shared_ptr<NodalKernelBase>> active_involved_kernels;

    if (_nkernel_warehouse->hasActiveBoundaryObjects(boundary_id, _tid))
    {
      // Loop over each NodalKernel to see if it's involved with the jvar
      const auto & objects = _nkernel_warehouse->getActiveBoundaryObjects(boundary_id, _tid);
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
          {
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

      if (_nkernel_warehouse->hasActiveBoundaryObjects(boundary_id, _tid))
      {
        Node * node = bnode->_node;
        if (node->processor_id() == _fe_problem.processor_id())
        {
          _fe_problem.reinitNodeFace(node, boundary_id, _tid);
          for (const auto & nodal_kernel : active_involved_kernels)
            nodal_kernel->computeOffDiagJacobian(jvar);

          _num_cached++;
        }
      }

      if (_num_cached == 20) // cache 20 nodes worth before adding into the jacobian
      {
        _num_cached = 0;
        // vectors are thread-safe, but matrices are not yet
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.addCachedJacobian(_tid);
      }
    }
  }
}

void
ComputeNodalKernelBCJacobiansThread::join(const ComputeNodalKernelBCJacobiansThread & /*y*/)
{
}

void
ComputeNodalKernelBCJacobiansThread::printGeneralExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid) || !_nkernel_warehouse->hasActiveBoundaryObjects())
    return;

  const auto & console = _fe_problem.console();
  const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
  console << "[DBG] Computing nodal kernel & boundary conditions contribution to the Jacobian on "
             "boundary nodes on "
          << execute_on << std::endl;
  console << "[DBG] Ordering on boundaries they are defined on:" << std::endl;
  console << _nkernel_warehouse->activeObjectsToFormattedString() << std::endl;
}
