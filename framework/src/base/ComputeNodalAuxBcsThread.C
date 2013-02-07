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

#include "ComputeNodalAuxBcsThread.h"

#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "AuxKernel.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeNodalAuxBcsThread::ComputeNodalAuxBcsThread(FEProblem & fe_problem,
                                                   AuxiliarySystem & sys,
                                                   std::vector<AuxWarehouse> & auxs) :
    _fe_problem(fe_problem),
    _sys(sys),
    _auxs(auxs)
{
}

// Splitting Constructor
ComputeNodalAuxBcsThread::ComputeNodalAuxBcsThread(ComputeNodalAuxBcsThread & x, Threads::split /*split*/) :
    _fe_problem(x._fe_problem),
    _sys(x._sys),
    _auxs(x._auxs)
{
}

void
ComputeNodalAuxBcsThread::operator() (const ConstBndNodeRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  for (ConstBndNodeRange::const_iterator nd = range.begin() ; nd != range.end(); ++nd)
  {
    const BndNode * bnode = *nd;

    BoundaryID boundary_id = bnode->_bnd_id;

    // prepare variables
    for (std::map<std::string, MooseVariable *>::iterator it = _sys._nodal_vars[_tid].begin(); it != _sys._nodal_vars[_tid].end(); ++it)
    {
      MooseVariable * var = it->second;
      var->prepare_aux();
    }

    if(_auxs[_tid].activeBCs(boundary_id).size() > 0)
    {
      Node * node = bnode->_node;

//      if(unlikely(_calculate_element_time))
//        startNodeTiming(node.id());

      if(node->processor_id() == libMesh::processor_id())
      {
        _fe_problem.reinitNodeFace(node, boundary_id, _tid);

        for (std::vector<AuxKernel *>::const_iterator aux_it = _auxs[_tid].activeBCs(boundary_id).begin();
            aux_it != _auxs[_tid].activeBCs(boundary_id).end();
            ++aux_it)
          (*aux_it)->compute();
      }

//      if(unlikely(_calculate_element_time))
//        stopNodeTiming(node.id());
    }

    // We are done, so update the solution vector
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      // update the solution vector
      for (std::map<std::string, MooseVariable *>::iterator it = _sys._nodal_vars[_tid].begin(); it != _sys._nodal_vars[_tid].end(); ++it)
      {
        MooseVariable * var = it->second;
        var->insert(_sys.solution());
      }
    }
  }
}

void
ComputeNodalAuxBcsThread::join(const ComputeNodalAuxBcsThread & /*y*/)
{
}
