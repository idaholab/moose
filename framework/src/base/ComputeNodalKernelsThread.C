//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeNodalKernelsThread.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "NodalKernel.h"

#include "libmesh/threads.h"

ComputeNodalKernelsThread::ComputeNodalKernelsThread(
    FEProblemBase & fe_problem, const MooseObjectWarehouse<NodalKernel> & nodal_kernels)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _nodal_kernels(nodal_kernels),
    _num_cached(0)
{
}

// Splitting Constructor
ComputeNodalKernelsThread::ComputeNodalKernelsThread(ComputeNodalKernelsThread & x,
                                                     Threads::split split)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _nodal_kernels(x._nodal_kernels),
    _num_cached(0)
{
}

void
ComputeNodalKernelsThread::pre()
{
  _num_cached = 0;
}

void
ComputeNodalKernelsThread::onNode(ConstNodeRange::const_iterator & node_it)
{
  const Node * node = *node_it;

  // prepare variables
  for (const auto & it : _aux_sys._nodal_vars[_tid])
  {
    MooseVariable * var = it.second;
    var->prepareAux();
  }

  _fe_problem.reinitNode(node, _tid);

  const std::set<SubdomainID> & block_ids = _aux_sys.mesh().getNodeBlockIds(*node);
  for (const auto & block : block_ids)
    if (_nodal_kernels.hasActiveBlockObjects(block, _tid))
    {
      const auto & objects = _nodal_kernels.getActiveBlockObjects(block, _tid);
      for (const auto & nodal_kernel : objects)
        nodal_kernel->computeResidual();
    }

  _num_cached++;

  if (_num_cached == 20) // Cache 20 nodes worth before adding into the residual
  {
    _num_cached = 0;
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _fe_problem.addCachedResidual(_tid);
  }
}

void
ComputeNodalKernelsThread::join(const ComputeNodalKernelsThread & /*y*/)
{
}
