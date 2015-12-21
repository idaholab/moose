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

// libMesh includes
#include "libmesh/threads.h"

// MOOSE includes
#include "ComputeNodalAuxBcsThread.h"
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "AuxKernel.h"


ComputeNodalAuxBcsThread::ComputeNodalAuxBcsThread(FEProblem & fe_problem,
                                                   AuxiliarySystem & sys,
                                                   const MooseObjectWarehouse<AuxKernel> & storage) :
    ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _aux_sys(sys),
    _storage(storage)
{
}

// Splitting Constructor
ComputeNodalAuxBcsThread::ComputeNodalAuxBcsThread(ComputeNodalAuxBcsThread & x, Threads::split split) :
    ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _storage(x._storage)
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

  Node * node = bnode->_node;

  if (node->processor_id() == _fe_problem.processor_id())
  {
    // Get a map of all active block restricted AuxKernel objects
    const std::map<BoundaryID, std::vector<MooseSharedPointer<AuxKernel> > > & kernels = _storage.getActiveBoundaryObjects(_tid);

    // Operate on the node BoundaryID only
    const std::map<BoundaryID, std::vector<MooseSharedPointer<AuxKernel> > >::const_iterator iter = kernels.find(boundary_id);
    if (iter != kernels.end())
    {
          _fe_problem.reinitNodeFace(node, boundary_id, _tid);

          for (std::vector<MooseSharedPointer<AuxKernel> >::const_iterator aux_it = iter->second.begin(); aux_it != iter->second.end(); ++aux_it)
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
