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
#include "threads.h"

ComputeNodalAuxVarsThread::ComputeNodalAuxVarsThread(FEProblem & fe_problem,
                                                     AuxiliarySystem & sys,
                                                     std::vector<AuxWarehouse> & auxs) :
    _fe_problem(fe_problem),
    _sys(sys),
    _auxs(auxs)
{
}

// Splitting Constructor
ComputeNodalAuxVarsThread::ComputeNodalAuxVarsThread(ComputeNodalAuxVarsThread & x, Threads::split /*split*/) :
    _fe_problem(x._fe_problem),
    _sys(x._sys),
    _auxs(x._auxs)
{
}

void
ComputeNodalAuxVarsThread::operator() (const ConstNodeRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  for (ConstNodeRange::const_iterator node_it = range.begin() ; node_it != range.end(); ++node_it)
  {
    const Node * node = *node_it;

    // prepare variables
    for (std::map<std::string, MooseVariable *>::iterator it = _sys._nodal_vars[_tid].begin(); it != _sys._nodal_vars[_tid].end(); ++it)
    {
      MooseVariable * var = it->second;
      var->prepare_aux();
    }

//  if(unlikely(_calculate_element_time))
//    startNodeTiming(node->id());

    _fe_problem.reinitNode(node, _tid);

    // compute global aux kernels
    for (std::vector<AuxKernel *>::const_iterator aux_it = _auxs[_tid].activeNodalKernels().begin();
        aux_it != _auxs[_tid].activeNodalKernels().end();
        ++aux_it)
      (*aux_it)->compute();

    const std::set<SubdomainID> & block_ids = _sys.mesh().getNodeBlockIds(*node);
    for (std::set<SubdomainID>::const_iterator block_it = block_ids.begin(); block_it != block_ids.end(); ++block_it)
    {
      for(std::vector<AuxKernel*>::const_iterator aux_it = _auxs[_tid].activeBlockNodalKernels(*block_it).begin();
          aux_it != _auxs[_tid].activeBlockNodalKernels(*block_it).end();
          ++aux_it)
        (*aux_it)->compute();
    }

//  if(unlikely(_calculate_element_time))
//    stopNodeTiming(node->id());

    // We are done, so update the solution vector
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (std::map<std::string, MooseVariable *>::iterator it = _sys._nodal_vars[_tid].begin(); it != _sys._nodal_vars[_tid].end(); ++it)
      {
        MooseVariable * var = it->second;
        var->insert(_sys.solution());
      }
    }
  }
}

void
ComputeNodalAuxVarsThread::join(const ComputeNodalAuxVarsThread & /*y*/)
{
}
