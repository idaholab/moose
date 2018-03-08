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
                                                         std::vector<JacobianBlock *> & blocks)
  : ComputeFullJacobianThread(fe_problem, blocks[0]->_jacobian /* have to pass something */),
    _blocks(blocks)
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
  std::vector<dof_id_type>
      dof_indices; // Do this out here to avoid creating and destroying it thousands of times

  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  for (const auto & block : _blocks)
  {
    const DofMap & dof_map = block->_precond_system.get_dof_map();
    dof_map.dof_indices(elem, dof_indices);

    _fe_problem.addJacobianBlock(
        block->_jacobian, block->_ivar, block->_jvar, dof_map, dof_indices, _tid);
  }
}
