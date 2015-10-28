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

#include "ComputeNodalKernelsThread.h"
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "NodalKernel.h"
#include "NodalKernelWarehouse.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeNodalKernelsThread::ComputeNodalKernelsThread(FEProblem & fe_problem,
                                                     AuxiliarySystem & sys,
                                                     std::vector<NodalKernelWarehouse> & nodal_kernels) :
    _fe_problem(fe_problem),
    _sys(sys),
    _nodal_kernels(nodal_kernels)
{
}

// Splitting Constructor
ComputeNodalKernelsThread::ComputeNodalKernelsThread(ComputeNodalKernelsThread & x, Threads::split /*split*/) :
    _fe_problem(x._fe_problem),
    _sys(x._sys),
    _nodal_kernels(x._nodal_kernels)
{
}

void
ComputeNodalKernelsThread::operator() (const ConstNodeRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  unsigned int num_cached = 0;

  for (ConstNodeRange::const_iterator node_it = range.begin() ; node_it != range.end(); ++node_it)
  {
    const Node * node = *node_it;

    // prepare variables
    for (std::map<std::string, MooseVariable *>::iterator it = _sys._nodal_vars[_tid].begin(); it != _sys._nodal_vars[_tid].end(); ++it)
    {
      MooseVariable * var = it->second;
      var->prepareAux();
    }

    _fe_problem.reinitNode(node, _tid);

    const std::set<SubdomainID> & block_ids = _sys.mesh().getNodeBlockIds(*node);
    for (std::set<SubdomainID>::const_iterator block_it = block_ids.begin(); block_it != block_ids.end(); ++block_it)
    {
      for (std::vector<MooseSharedPointer<NodalKernel> >::const_iterator nodal_kernel_it = _nodal_kernels[_tid].activeBlockNodalKernels(*block_it).begin();
          nodal_kernel_it != _nodal_kernels[_tid].activeBlockNodalKernels(*block_it).end();
          ++nodal_kernel_it)
        (*nodal_kernel_it)->computeResidual();
    }

    num_cached++;

    if (num_cached % 20 == 0) // Cache 20 nodes worth before adding into the residual
    {
      num_cached = 0;
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      _fe_problem.addCachedResidual(_tid);
    }
  }
}

void
ComputeNodalKernelsThread::join(const ComputeNodalKernelsThread & /*y*/)
{
}
