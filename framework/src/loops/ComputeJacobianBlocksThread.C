//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeJacobianBlocksThread.h"

// MOOSE includes
#include "DGKernel.h"
#include "FEProblem.h"
#include "KernelBase.h"
#include "IntegratedBC.h"

#include "libmesh/threads.h"
#include "libmesh/dof_map.h"

ComputeJacobianBlocksThread::ComputeJacobianBlocksThread(FEProblemBase & fe_problem,
                                                         std::vector<JacobianBlock *> & blocks,
                                                         const std::set<TagID> & tags)
  : ComputeFullJacobianThread(fe_problem, tags), _blocks(blocks)
{
}

// Splitting Constructor
ComputeJacobianBlocksThread::ComputeJacobianBlocksThread(ComputeJacobianBlocksThread & x,
                                                         Threads::split split)
  : ComputeFullJacobianThread(x, split), _blocks(x._blocks)
{
}

ComputeJacobianBlocksThread::~ComputeJacobianBlocksThread() {}

void
ComputeJacobianBlocksThread::postElement(const Elem * elem)
{
  _dof_indices.clear();

  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  for (const auto & block : _blocks)
  {
    const auto & dof_map = block->_precond_system.get_dof_map();
    dof_map.dof_indices(elem, _dof_indices);

    _fe_problem.addJacobianBlockTags(
        block->_jacobian, block->_ivar, block->_jvar, dof_map, _dof_indices, _tags, _tid);
  }
}

void
ComputeJacobianBlocksThread::postInternalSide(const Elem * elem, unsigned int side)
{
  if (_dg_warehouse->hasActiveBlockObjects(_subdomain, _tid))
  {
    _dof_indices.clear();
    _dof_neighbor_indices.clear();

    // Pointer to the neighbor we are currently working on.
    const Elem * neighbor = elem->neighbor_ptr(side);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    // Get the global id of the element and the neighbor
    const auto elem_id = elem->id(), neighbor_id = neighbor->id();

    if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) ||
        (neighbor->level() < elem->level()))
      for (const auto & block : _blocks)
      {
        const auto & dof_map = block->_precond_system.get_dof_map();
        dof_map.dof_indices(elem, _dof_indices);
        dof_map.dof_indices(neighbor, _dof_neighbor_indices);

        _fe_problem.addJacobianNeighbor(block->_jacobian,
                                        block->_ivar,
                                        block->_jvar,
                                        dof_map,
                                        _dof_indices,
                                        _dof_neighbor_indices,
                                        _tid);
      }
  }
}
