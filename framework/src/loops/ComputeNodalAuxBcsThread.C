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

ComputeNodalAuxBcsThread::ComputeNodalAuxBcsThread(FEProblemBase & fe_problem,
                                                   const MooseObjectWarehouse<AuxKernel> & storage)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _storage(storage)
{
}

// Splitting Constructor
ComputeNodalAuxBcsThread::ComputeNodalAuxBcsThread(ComputeNodalAuxBcsThread & x,
                                                   Threads::split split)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _storage(x._storage)
{
}

void
ComputeNodalAuxBcsThread::onNode(ConstBndNodeRange::const_iterator & node_it)
{
  const BndNode * bnode = *node_it;

  BoundaryID boundary_id = bnode->_bnd_id;

  // prepare variables
  for (const auto & it : _aux_sys._nodal_vars[_tid])
  {
    MooseVariable * var = it.second;
    var->prepareAux();
  }

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
    for (const auto & it : _aux_sys._nodal_vars[_tid])
    {
      MooseVariable * var = it.second;
      var->insert(_aux_sys.solution());
    }
  }
}

void
ComputeNodalAuxBcsThread::join(const ComputeNodalAuxBcsThread & /*y*/)
{
}
