/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ComputeJacobianBlocksThread.h"
#include "FEProblem.h"
#include "KernelBase.h"
#include "IntegratedBC.h"
#include "DGKernel.h"
// libmesh includes
#include "libmesh/threads.h"

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
