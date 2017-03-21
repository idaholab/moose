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

#include "ComputeNodalAuxVarsThread.h"
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "AuxKernel.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeNodalAuxVarsThread::ComputeNodalAuxVarsThread(
    FEProblemBase & fe_problem, const MooseObjectWarehouse<AuxKernel> & storage)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _storage(storage)
{
}

// Splitting Constructor
ComputeNodalAuxVarsThread::ComputeNodalAuxVarsThread(ComputeNodalAuxVarsThread & x,
                                                     Threads::split split)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _storage(x._storage)
{
}

void
ComputeNodalAuxVarsThread::onNode(ConstNodeRange::const_iterator & node_it)
{
  const Node * node = *node_it;

  // prepare variables
  for (const auto & it : _aux_sys._nodal_vars[_tid])
  {
    MooseVariable * var = it.second;
    var->prepareAux();
  }

  _fe_problem.reinitNode(node, _tid);

  // Get a map of all active block restricted AuxKernel objects
  const auto & block_kernels = _storage.getActiveBlockObjects(_tid);

  // Loop over all SubdomainIDs for the curnent node, if an AuxKernel is active on this block then
  // compute it.
  const auto & block_ids = _aux_sys.mesh().getNodeBlockIds(*node);
  for (const auto & block : block_ids)
  {
    const auto iter = block_kernels.find(block);

    if (iter != block_kernels.end())
      for (const auto & aux : iter->second)
        aux->compute();
  }

  // We are done, so update the solution vector
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & it : _aux_sys._nodal_vars[_tid])
    {
      MooseVariable * var = it.second;
      var->insert(_aux_sys.solution());
    }
  }
}

void
ComputeNodalAuxVarsThread::join(const ComputeNodalAuxVarsThread & /*y*/)
{
}
