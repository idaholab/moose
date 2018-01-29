//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeNodalKernelBCJacobiansThread.h"

// MOOSE includes
#include "Assembly.h"
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "MooseVariable.h"
#include "NodalKernel.h"

#include "libmesh/threads.h"

ComputeNodalKernelBCJacobiansThread::ComputeNodalKernelBCJacobiansThread(
    FEProblemBase & fe_problem,
    const MooseObjectWarehouse<NodalKernel> & nodal_kernels,
    SparseMatrix<Number> & jacobian)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _nodal_kernels(nodal_kernels),
    _jacobian(jacobian),
    _num_cached(0)
{
}

// Splitting Constructor
ComputeNodalKernelBCJacobiansThread::ComputeNodalKernelBCJacobiansThread(
    ComputeNodalKernelBCJacobiansThread & x, Threads::split split)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _nodal_kernels(x._nodal_kernels),
    _jacobian(x._jacobian),
    _num_cached(0)
{
}

void
ComputeNodalKernelBCJacobiansThread::pre()
{
  _num_cached = 0;
}

void
ComputeNodalKernelBCJacobiansThread::onNode(ConstBndNodeRange::const_iterator & node_it)
{
  const BndNode * bnode = *node_it;

  BoundaryID boundary_id = bnode->_bnd_id;

  std::vector<std::pair<MooseVariable *, MooseVariable *>> & ce = _fe_problem.couplingEntries(_tid);
  for (const auto & it : ce)
  {
    MooseVariable & ivariable = *(it.first);
    MooseVariable & jvariable = *(it.second);

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    // The NodalKernels that are active and are coupled to the jvar in question
    std::vector<std::shared_ptr<NodalKernel>> active_involved_kernels;

    if (_nodal_kernels.hasActiveBoundaryObjects(boundary_id, _tid))
    {
      // Loop over each NodalKernel to see if it's involved with the jvar
      const auto & objects = _nodal_kernels.getActiveBoundaryObjects(boundary_id, _tid);
      for (const auto & nodal_kernel : objects)
      {
        if (nodal_kernel->variable().number() == ivar)
        {
          // If this NodalKernel is acting on the jvar add it to the list and short-circuit the
          // loop
          if (nodal_kernel->variable().number() == jvar)
          {
            active_involved_kernels.push_back(nodal_kernel);
            continue;
          }

          // See if this NodalKernel is coupled to the jvar
          const std::vector<MooseVariable *> & coupled_vars = nodal_kernel->getCoupledMooseVars();
          for (const auto & var : coupled_vars)
          {
            if (var->number() == jvar)
            {
              active_involved_kernels.push_back(nodal_kernel);
              break; // It only takes one
            }
          }
        }
      }
    }

    // Did we find any NodalKernels coupled to this jvar?
    if (!active_involved_kernels.empty())
    {
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
          for (const auto & nodal_kernel : active_involved_kernels)
            nodal_kernel->computeOffDiagJacobian(jvar);

          _num_cached++;
        }
      }

      if (_num_cached == 20) // cache 20 nodes worth before adding into the jacobian
      {
        _num_cached = 0;
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.assembly(_tid).addCachedJacobianContributions(_jacobian);
      }
    }
  }
}

void
ComputeNodalKernelBCJacobiansThread::join(const ComputeNodalKernelBCJacobiansThread & /*y*/)
{
}
