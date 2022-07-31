//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
ComputeNodalAuxBcsThread<AuxKernelType>::ComputeNodalAuxBcsThread(
    FEProblemBase & fe_problem,
    const MooseObjectWarehouse<AuxKernelType> & storage,
    const std::vector<std::vector<MooseVariableFEBase *>> & vars)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _storage(storage),
    _aux_vars(vars)
{
}

// Splitting Constructor
template <typename AuxKernelType>
ComputeNodalAuxBcsThread<AuxKernelType>::ComputeNodalAuxBcsThread(ComputeNodalAuxBcsThread & x,
                                                                  Threads::split split)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _storage(x._storage),
    _aux_vars(x._aux_vars)
{
}

template <typename AuxKernelType>
void
ComputeNodalAuxBcsThread<AuxKernelType>::onNode(ConstBndNodeRange::const_iterator & node_it)
{
  const BndNode * bnode = *node_it;

  BoundaryID boundary_id = bnode->_bnd_id;

  // prepare variables
  for (auto * var : _aux_vars[_tid])
    var->prepareAux();

  Node * node = bnode->_node;

  if (node->processor_id() == _fe_problem.processor_id())
  {
    // Get a map of all active block restricted AuxKernel objects
    const auto & kernels = _storage.getActiveBoundaryObjects(_tid);

    // Operate on the node BoundaryID only
    const auto iter = kernels.find(boundary_id);
    if (iter != kernels.end())
    {
      _fe_problem.reinitNodeFace(node, boundary_id, _tid);

      for (const auto & aux : iter->second)
        aux->compute();
    }
  }

  // We are done, so update the solution vector
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    // update the solution vector
    for (auto * var : _aux_vars[_tid])
      var->insert(_aux_sys.solution());
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
  if (_fe_problem.shouldPrintExecution() && _storage.hasActiveObjects())
  {
    auto console = _fe_problem.console();
    auto execute_on = _fe_problem.getCurrentExecuteOnFlag();
    console << "[DBG] Executing nodal auxiliary kernels on boundary nodes on " << execute_on
            << std::endl;
    console << "[DBG] Ordering of the kernels on each boundary they are defined on:" << std::endl;
    console << "[DBG] " << _storage.activeObjectsToString() << std::endl;
  }
}

template class ComputeNodalAuxBcsThread<AuxKernel>;
template class ComputeNodalAuxBcsThread<VectorAuxKernel>;
template class ComputeNodalAuxBcsThread<ArrayAuxKernel>;
