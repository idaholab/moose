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
#include "Problem.h"
#include "MProblem.h"

// libmesh includes
#include "threads.h"

ComputeNodalAuxVarsThread::ComputeNodalAuxVarsThread(Problem & problem,
                                                     AuxiliarySystem & sys,
                                                     std::vector<AuxWarehouse> & auxs) :
    _problem(problem),
    _sys(sys),
    _auxs(auxs)
{
}

// Splitting Constructor
ComputeNodalAuxVarsThread::ComputeNodalAuxVarsThread(ComputeNodalAuxVarsThread & x, Threads::split split) :
    _problem(x._problem),
    _sys(x._sys),
    _auxs(x._auxs)
{
}

void
ComputeNodalAuxVarsThread::operator() (const ConstNodeRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  // prepare variables
//  for (int vn = 0; vn < _sys.nVariables(); ++vn)
//  {
//    const std::string & var_name = _sys.sys().variable_name(vn);
//    MooseVariable & var = _sys.getVariable(_tid, var_name);
//    var.prepare_nodal(range.size());
//  }

  for (ConstNodeRange::const_iterator node_it = range.begin() ; node_it != range.end(); ++node_it)
  {
    const Node * node = *node_it;

    // prepare variables
    for (std::map<std::string, MooseVariable *>::iterator it = _sys._nodal_vars[_tid].begin(); it != _sys._nodal_vars[_tid].end(); ++it)
    {
      MooseVariable * var = it->second;
      var->prepare_aux();

//      {
//        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
//        std::cerr << "[" << libMesh::processor_id() << "]: Thread #" << _tid << " var " << var->number() << " size = " << range.size() << std::endl;
//      }
    }

//    {
//      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
//      std::cerr << "[" << libMesh::processor_id() << "]: Thread #" << _tid << " is doing node " << node->id() << std::endl;
//    }

//  if(unlikely(_calculate_element_time))
//    startNodeTiming(node->id());

    _problem.reinitNode(node, _tid);

    // compute global aux kernels
    AuxKernelIterator aux_begin = _auxs[_tid].activeNodalAuxKernelsBegin();
    AuxKernelIterator aux_end = _auxs[_tid].activeNodalAuxKernelsEnd();
    for(AuxKernelIterator aux_it = aux_begin; aux_it != aux_end; ++aux_it)
      (*aux_it)->compute();

    const std::set<subdomain_id_type> & block_ids = _sys.mesh().getNodeBlockIds(*node);
    for (std::set<subdomain_id_type>::const_iterator block_it = block_ids.begin(); block_it != block_ids.end(); ++block_it)
    {
//      std::cerr << "blk = " << (int) *block_it << std::endl;
      AuxKernelIterator block_nodal_aux_begin = _auxs[_tid].activeBlockNodalAuxKernelsBegin(*block_it);
      AuxKernelIterator block_nodal_aux_end = _auxs[_tid].activeBlockNodalAuxKernelsEnd(*block_it);

      for(AuxKernelIterator aux_it = block_nodal_aux_begin; aux_it != block_nodal_aux_end; ++aux_it)
        (*aux_it)->compute();
    }

//  if(unlikely(_calculate_element_time))
//    stopNodeTiming(node->id());

    // We are done, so update the solution vector
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
//      for (int vn = 0; vn < _sys.nVariables(); ++vn)
//      {
//        const std::string & var_name = _sys.sys().variable_name(vn);
//        MooseVariable & var = _sys.getVariable(_tid, var_name);
//        var.insert(_sys.solution());
//      }
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
