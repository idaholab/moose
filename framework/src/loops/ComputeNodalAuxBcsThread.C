//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/threads.h"

// MOOSE includes
#include "ComputeNodalAuxBcsThread.h"
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "AuxKernel.h"

template <typename AuxKernelType>
ComputeNodalAuxBcsThread<AuxKernelType>::ComputeNodalAuxBcsThread(FEProblemBase & fe_problem,
                                                                  const TheWarehouse::Query & query)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _query(query),
    _query_boundary(_query)
{
}

// Splitting Constructor
template <typename AuxKernelType>
ComputeNodalAuxBcsThread<AuxKernelType>::ComputeNodalAuxBcsThread(ComputeNodalAuxBcsThread & x,
                                                                  Threads::split split)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _query(x._query),
    _query_boundary(x._query_boundary)
{
}

template <typename AuxKernelType>
void
ComputeNodalAuxBcsThread<AuxKernelType>::onNode(ConstBndNodeRange::const_iterator & node_it)
{
  const BndNode * bnode = *node_it;

  BoundaryID boundary_id = bnode->_bnd_id;

  Node * node = bnode->_node;

  if (node->processor_id() == _fe_problem.processor_id())
  {
    // Only kernels actually restricted to this boundary - unrestricted ones are already handled
    // by the block-based nodal loop.
    std::vector<AuxKernelType *> kernels;
    _query_boundary.queryInto(
        kernels, _tid, std::make_tuple(boundary_id, /*must_be_restricted=*/true));

    if (kernels.size())
    {
      _fe_problem.reinitNodeFace(node, boundary_id, _tid);

      for (const auto & aux : kernels)
      {
        aux->compute();
        // This is the same conditional check that the aux kernel performs internally before calling
        // computeValue and _var.setNodalValue. We don't want to attempt to insert into the solution
        // if we don't actually have any dofs on this node
        if (aux->variable().isNodalDefined())
          aux->variable().insert(_aux_sys.solution());
      }
    }
  }
}

template <typename AuxKernelType>
void
ComputeNodalAuxBcsThread<AuxKernelType>::join(const ComputeNodalAuxBcsThread & /*y*/)
{
}

template <typename AuxKernelType>
void
ComputeNodalAuxBcsThread<AuxKernelType>::printGeneralExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid))
    return;

  std::vector<AuxKernelType *> all_kernels;
  // clone the query until we have a const query, see #32362
  _query.clone().condition<AttribThread>(_tid).queryInto(all_kernels);
  if (all_kernels.empty())
    return;

  const auto & console = _fe_problem.console();
  const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
  console << "[DBG] Executing nodal auxiliary kernels on boundary nodes on " << execute_on
          << std::endl;
  console << "[DBG] Ordering of the kernels on each boundary they are defined on:" << std::endl;
  printExecutionOrdering<AuxKernelType>(all_kernels, /*print_header=*/false);
}

template class ComputeNodalAuxBcsThread<AuxKernel>;
template class ComputeNodalAuxBcsThread<VectorAuxKernel>;
template class ComputeNodalAuxBcsThread<ArrayAuxKernel>;
