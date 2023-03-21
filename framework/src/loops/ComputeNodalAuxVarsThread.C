//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
    FEProblemBase & fe_problem,
    const MooseObjectWarehouse<AuxKernelType> & storage,
    const std::vector<std::vector<MooseVariableFEBase *>> & vars)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _storage(storage),
    _aux_vars(vars)
{
}

// Splitting Constructor
template <typename AuxKernelType>
ComputeNodalAuxVarsThread<AuxKernelType>::ComputeNodalAuxVarsThread(ComputeNodalAuxVarsThread & x,
                                                                    Threads::split split)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _storage(x._storage),
    _aux_vars(x._aux_vars)
{
}

template <typename AuxKernelType>
void
ComputeNodalAuxVarsThread<AuxKernelType>::subdomainChanged()
{
  std::set<TagID> needed_vector_tags;
  std::set<TagID> needed_matrix_tags;

  const auto & block_kernels = _storage.getActiveBlockObjects(_tid);

  for (const auto & block : _block_ids)
  {
    const auto iter = block_kernels.find(block);

    if (iter != block_kernels.end())
      for (const auto & aux : iter->second)
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

  // prepare variables
  for (auto * var : _aux_vars[_tid])
    var->prepareAux();

  _fe_problem.reinitNode(node, _tid);

  // Get a map of all active block restricted AuxKernel objects
  const auto & block_kernels = _storage.getActiveBlockObjects(_tid);

  // Loop over all SubdomainIDs for the current node, if an AuxKernel is active on this block then
  // compute it.
  for (const auto & block : block_ids)
  {
    const auto iter = block_kernels.find(block);

    if (iter != block_kernels.end())
      for (const auto & aux : iter->second)
      {
        aux->compute();

        // update the aux solution vector if writable coupled variables are used
        if (aux->hasWritableCoupledVariables())
        {
          Threads::spin_mutex::scoped_lock lock(writable_variable_mutex);
          for (auto * var : aux->getWritableCoupledVariables())
          {
            // insert into the global solution vector
            var->insert(_aux_sys.solution());
            var->prepareAux();
          }

          // make solution values available for dependent AuxKernels
          aux->variable().insert(_aux_sys.solution());
          aux->variable().prepareAux();
          _fe_problem.reinitNode(node, _tid);
        }
      }
  }

  // We are done, so update the solution vector
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (auto * var : _aux_vars[_tid])
      var->insert(_aux_sys.solution());
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
  if (_fe_problem.shouldPrintExecution(_tid) && _storage.hasActiveObjects())
  {
    auto console = _fe_problem.console();
    auto execute_on = _fe_problem.getCurrentExecuteOnFlag();
    console << "[DBG] Beginning nodal loop of nodal auxiliary kernels on " << execute_on
            << std::endl;
    console << "[DBG] Ordering of the kernels on each block they are defined on" << std::endl;
    console << _storage.activeObjectsToFormattedString() << std::endl;
  }
}

template class ComputeNodalAuxVarsThread<AuxKernel>;
template class ComputeNodalAuxVarsThread<VectorAuxKernel>;
template class ComputeNodalAuxVarsThread<ArrayAuxKernel>;
