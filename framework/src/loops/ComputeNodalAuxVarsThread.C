//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeNodalAuxVarsThread.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "AuxKernel.h"
#include "FEProblem.h"
#include "MooseMesh.h"

#include "libmesh/threads.h"

template <typename AuxKernelType>
Threads::spin_mutex ComputeNodalAuxVarsThread<AuxKernelType>::writable_variable_mutex;

template <typename AuxKernelType>
ComputeNodalAuxVarsThread<AuxKernelType>::ComputeNodalAuxVarsThread(
    FEProblemBase & fe_problem, const TheWarehouse::Query & query)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    // Note: the main query is only here to provide the cache. Maybe we should just pass a cache
    _query(query),
    _query_subdomain(_query)
{
}

// Splitting Constructor
template <typename AuxKernelType>
ComputeNodalAuxVarsThread<AuxKernelType>::ComputeNodalAuxVarsThread(ComputeNodalAuxVarsThread & x,
                                                                    Threads::split split)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _query(x._query),
    _query_subdomain(x._query_subdomain)
{
}

template <typename AuxKernelType>
void
ComputeNodalAuxVarsThread<AuxKernelType>::subdomainChanged()
{
  std::set<TagID> needed_vector_tags;
  std::set<TagID> needed_matrix_tags;

  for (const auto & block : _block_ids)
  {
    std::vector<AuxKernelType *> block_kernels;
    _query_subdomain.queryInto(block_kernels, _tid, block);

    if (block_kernels.size())
      for (const auto & aux : block_kernels)
      {
        auto & matrix_tags = aux->getFEVariableCoupleableMatrixTags();
        needed_matrix_tags.insert(matrix_tags.begin(), matrix_tags.end());
        auto & vector_tags = aux->getFEVariableCoupleableVectorTags();
        needed_vector_tags.insert(vector_tags.begin(), vector_tags.end());
      }
  }

  _fe_problem.setActiveFEVariableCoupleableMatrixTags(needed_matrix_tags, _tid);
  _fe_problem.setActiveFEVariableCoupleableVectorTags(needed_vector_tags, _tid);
}

template <typename AuxKernelType>
void
ComputeNodalAuxVarsThread<AuxKernelType>::onNode(ConstNodeRange::const_iterator & node_it)
{
  const Node * node = *node_it;

  const auto & block_ids = _aux_sys.mesh().getNodeBlockIds(*node);

  if (_block_ids != block_ids)
  {
    _block_ids.clear();
    _block_ids.insert(block_ids.begin(), block_ids.end());
    subdomainChanged();
  }

  _fe_problem.reinitNode(node, _tid);

  // Loop over all SubdomainIDs for the current node, if an AuxKernel is active on this block then
  // compute it.
  for (const auto & block : _block_ids)
  {
    std::vector<AuxKernelType *> block_kernels;
    _query_subdomain.queryInto(block_kernels, _tid, block);

    if (block_kernels.size())
      for (const auto & aux : block_kernels)
      {
        aux->compute();
        // This is the same conditional check that the aux kernel performs internally before calling
        // computeValue and _var.setNodalValue. We don't want to attempt to insert into the solution
        // if we don't actually have any dofs on this node
        if (aux->variable().isNodalDefined())
          aux->variable().insert(_aux_sys.solution());

        // update the aux solution vector if writable coupled variables are used
        if (aux->hasWritableCoupledVariables())
        {
          for (auto * var : aux->getWritableCoupledVariables())
            if (var->isNodalDefined())
              // insert into the global solution vector
              var->insert(_aux_sys.solution());

          // make solution values available for dependent AuxKernels
          _fe_problem.reinitNode(node, _tid);
        }
      }
  }
}

template <typename AuxKernelType>
void
ComputeNodalAuxVarsThread<AuxKernelType>::post()
{
  _fe_problem.clearActiveFEVariableCoupleableVectorTags(_tid);
  _fe_problem.clearActiveFEVariableCoupleableMatrixTags(_tid);
}

template <typename AuxKernelType>
void
ComputeNodalAuxVarsThread<AuxKernelType>::join(const ComputeNodalAuxVarsThread & /*y*/)
{
}

template <typename AuxKernelType>
void
ComputeNodalAuxVarsThread<AuxKernelType>::printGeneralExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid))
    return;

  std::vector<AuxKernelType *> all_kernels;
  // clone the query until we have a const query, see #32362
  // This is "general" execution ordering so we query for all subdomains
  _query.clone().condition<AttribThread>(_tid).queryInto(all_kernels);
  if (all_kernels.empty())
    return;

  const auto & console = _fe_problem.console();
  const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
  console << "[DBG] Beginning nodal loop of nodal " << MooseUtils::prettyCppType<AuxKernelType>()
          << " on " << execute_on << std::endl;
  console << "[DBG] Ordering of the kernels on each block they are defined on:" << std::endl;
  // TODO Check that all objects are active at this point
  printExecutionOrdering<AuxKernelType>(all_kernels, "auxiliary kernels");
}

template class ComputeNodalAuxVarsThread<AuxKernel>;
template class ComputeNodalAuxVarsThread<VectorAuxKernel>;
template class ComputeNodalAuxVarsThread<ArrayAuxKernel>;
