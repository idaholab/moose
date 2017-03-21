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

#include "ComputeNodalKernelJacobiansThread.h"
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "NodalKernel.h"
#include "Assembly.h"

// libmesh includes
#include "libmesh/sparse_matrix.h"

ComputeNodalKernelJacobiansThread::ComputeNodalKernelJacobiansThread(
    FEProblemBase & fe_problem,
    const MooseObjectWarehouse<NodalKernel> & nodal_kernels,
    SparseMatrix<Number> & jacobian)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _nodal_kernels(nodal_kernels),
    _jacobian(jacobian),
    _num_cached(0)
{
}

// Splitting Constructor
ComputeNodalKernelJacobiansThread::ComputeNodalKernelJacobiansThread(
    ComputeNodalKernelJacobiansThread & x, Threads::split split)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _nodal_kernels(x._nodal_kernels),
    _jacobian(x._jacobian),
    _num_cached(0)
{
}

void
ComputeNodalKernelJacobiansThread::pre()
{
  _num_cached = 0;
}

void
ComputeNodalKernelJacobiansThread::onNode(ConstNodeRange::const_iterator & node_it)
{
  const Node * node = *node_it;

  std::vector<std::pair<MooseVariable *, MooseVariable *>> & ce = _fe_problem.couplingEntries(_tid);
  for (const auto & it : ce)
  {
    MooseVariable & ivariable = *(it.first);
    MooseVariable & jvariable = *(it.second);

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    // The NodalKernels that are active and are coupled to the jvar in question
    std::vector<std::shared_ptr<NodalKernel>> active_involved_kernels;

    const std::set<SubdomainID> & block_ids = _aux_sys.mesh().getNodeBlockIds(*node);
    for (const auto & block : block_ids)
    {
      if (_nodal_kernels.hasActiveBlockObjects(block, _tid))
      {
        // Loop over each NodalKernel to see if it's involved with the jvar
        const auto & objects = _nodal_kernels.getActiveBlockObjects(block, _tid);
        for (const auto & nodal_kernel : objects)
        {
          // If this NodalKernel isn't operating on this ivar... skip it
          if (nodal_kernel->variable().number() != ivar)
            break;

          // If this NodalKernel is acting on the jvar add it to the list and short-circuit the loop
          if (nodal_kernel->variable().number() == jvar)
          {
            active_involved_kernels.push_back(nodal_kernel);
            continue;
          }

          // See if this NodalKernel is coupled to the jvar
          const std::vector<MooseVariable *> & coupled_vars = nodal_kernel->getCoupledMooseVars();
          for (const auto & var : coupled_vars)
            if (var->number() == jvar)
            {
              active_involved_kernels.push_back(nodal_kernel);
              break; // It only takes one
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

      _fe_problem.reinitNode(node, _tid);

      for (const auto & nodal_kernel : active_involved_kernels)
        nodal_kernel->computeOffDiagJacobian(jvar);

      _num_cached++;

      if (_num_cached == 20) // Cache 20 nodes worth before adding into the residual
      {
        _num_cached = 0;
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.assembly(_tid).addCachedJacobianContributions(_jacobian);
      }
    }
  }
}

void
ComputeNodalKernelJacobiansThread::join(const ComputeNodalKernelJacobiansThread & /*y*/)
{
}
