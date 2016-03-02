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

// MOOSE includes
#include "ComputeNodalUserObjectsThread.h"
#include "FEProblem.h"
#include "NodalUserObject.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeNodalUserObjectsThread::ComputeNodalUserObjectsThread(FEProblem & fe_problem, const MooseObjectWarehouse<NodalUserObject> & user_objects) :
    ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(fe_problem),
    _user_objects(user_objects)
{
}

// Splitting Constructor
ComputeNodalUserObjectsThread::ComputeNodalUserObjectsThread(ComputeNodalUserObjectsThread & x, Threads::split split) :
    ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _user_objects(x._user_objects)
{
}

ComputeNodalUserObjectsThread::~ComputeNodalUserObjectsThread()
{
}

void
ComputeNodalUserObjectsThread::onNode(ConstNodeRange::const_iterator & node_it)
{
  const Node * node = *node_it;
  _fe_problem.reinitNode(node, _tid);

  // Boundary Restricted
  std::vector<BoundaryID> nodeset_ids;
  _fe_problem.mesh().getMesh().get_boundary_info().boundary_ids(node, nodeset_ids);
  for (std::vector<BoundaryID>::const_iterator bnd_it = nodeset_ids.begin(); bnd_it != nodeset_ids.end(); ++bnd_it)
  {
    if (_user_objects.hasActiveBoundaryObjects(*bnd_it, _tid))
    {
      const std::vector<MooseSharedPointer<NodalUserObject> > & objects = _user_objects.getActiveBoundaryObjects(*bnd_it, _tid);
      for (std::vector<MooseSharedPointer<NodalUserObject> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
        (*it)->execute();
    }
  }

  // Block Restricted
  // NodalUserObjects may be block restricted, in this case by default the execute() method is called for
  // each subdomain that the node "belongs". This may be disabled in the NodalUserObject by setting
  // "unique_node_execute = true".

  // To inforce the unique execution this vector is populated and checked if the unique flag is enabled.
  std::vector<MooseSharedPointer<NodalUserObject> > computed;

  const std::set<SubdomainID> & block_ids = _fe_problem.mesh().getNodeBlockIds(*node);
  for (std::set<SubdomainID>::const_iterator blk_it = block_ids.begin(); blk_it != block_ids.end(); ++blk_it)
  {
    if (_user_objects.hasActiveBlockObjects(*blk_it, _tid))
    {
      const std::vector<MooseSharedPointer<NodalUserObject> > & objects = _user_objects.getActiveBlockObjects(*blk_it, _tid);
      for (std::vector<MooseSharedPointer<NodalUserObject> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
      {
        if (!(*it)->isUniqueNodeExecute() || std::count(computed.begin(), computed.end(), *it) == 0)
        {
          (*it)->execute();
          computed.push_back(*it);
        }
      }
    }
  }
}

void
ComputeNodalUserObjectsThread::join(const ComputeNodalUserObjectsThread & /*y*/)
{
}
