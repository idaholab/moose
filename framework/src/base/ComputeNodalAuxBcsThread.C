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
    ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _aux_sys(sys),
    _auxs(auxs)
{
}

// Splitting Constructor
ComputeNodalAuxBcsThread::ComputeNodalAuxBcsThread(ComputeNodalAuxBcsThread & x, Threads::split split) :
    ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _auxs(x._auxs)
{
}

void
ComputeNodalAuxBcsThread::onNode(ConstBndNodeRange::const_iterator & node_it)
{
  const BndNode * bnode = *node_it;

  BoundaryID boundary_id = bnode->_bnd_id;

  // prepare variables
  for (std::map<std::string, MooseVariable *>::iterator it = _aux_sys._nodal_vars[_tid].begin(); it != _aux_sys._nodal_vars[_tid].end(); ++it)
  {
    MooseVariable * var = it->second;
    var->prepareAux();
  }

  if (_auxs[_tid].activeBCs(boundary_id).size() > 0)
  {
    Node * node = bnode->_node;

    if (node->processor_id() == _fe_problem.processor_id())
    {
      _fe_problem.reinitNodeFace(node, boundary_id, _tid);

      for (std::vector<AuxKernel *>::const_iterator aux_it = _auxs[_tid].activeBCs(boundary_id).begin();
           aux_it != _auxs[_tid].activeBCs(boundary_id).end();
           ++aux_it)
        (*aux_it)->compute();
    }
  }

  // We are done, so update the solution vector
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    // update the solution vector
    for (std::map<std::string, MooseVariable *>::iterator it = _aux_sys._nodal_vars[_tid].begin(); it != _aux_sys._nodal_vars[_tid].end(); ++it)
    {
      MooseVariable * var = it->second;
      var->insert(_aux_sys.solution());
    }
  }
}

void
ComputeNodalAuxBcsThread::join(const ComputeNodalAuxBcsThread & /*y*/)
{
}
