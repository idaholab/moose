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

#include "ComputeNodalKernelBcsThread.h"

#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "NodalKernel.h"

// libmesh includes
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
