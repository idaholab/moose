//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeNodalKernelBcsThread.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "MooseVariable.h"
#include "NodalKernel.h"

#include "libmesh/threads.h"

ComputeNodalKernelBcsThread::ComputeNodalKernelBcsThread(
    FEProblemBase & fe_problem, const MooseObjectWarehouse<NodalKernel> & nodal_kernels)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _nodal_kernels(nodal_kernels),
    _num_cached(0)
{
}

// Splitting Constructor
ComputeNodalKernelBcsThread::ComputeNodalKernelBcsThread(ComputeNodalKernelBcsThread & x,
                                                         Threads::split split)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _nodal_kernels(x._nodal_kernels),
    _num_cached(0)
{
}

void
ComputeNodalKernelBcsThread::pre()
{
  _num_cached = 0;
}

void
ComputeNodalKernelBcsThread::onNode(ConstBndNodeRange::const_iterator & node_it)
{
  const BndNode * bnode = *node_it;

  BoundaryID boundary_id = bnode->_bnd_id;

  // prepare variables
  for (const auto & it : _aux_sys._nodal_vars[_tid])
  {
    MooseVariable * var = it.second;
    var->prepareAux();
  }

  if (_nodal_kernels.hasActiveBoundaryObjects(boundary_id, _tid))
  {
    Node * node = bnode->_node;
    if (node->processor_id() == _fe_problem.processor_id())
    {
      _fe_problem.reinitNodeFace(node, boundary_id, _tid);
      const auto & objects = _nodal_kernels.getActiveBoundaryObjects(boundary_id, _tid);
      for (const auto & nodal_kernel : objects)
        nodal_kernel->computeResidual();

      _num_cached++;
    }
  }

  if (_num_cached == 20) // cache 20 nodes worth before adding into the residual
  {
    _num_cached = 0;
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _fe_problem.addCachedResidual(_tid);
  }
}

void
ComputeNodalKernelBcsThread::join(const ComputeNodalKernelBcsThread & /*y*/)
{
}
