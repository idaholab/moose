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

#include "ComputeNodalUserObjectsThread.h"

#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "NodalUserObject.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeNodalUserObjectsThread::ComputeNodalUserObjectsThread(FEProblem & fe_problem, std::vector<UserObjectWarehouse> & user_objects, UserObjectWarehouse::GROUP group) :
    ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(fe_problem),
    _user_objects(user_objects),
    _group(group)
{
}

// Splitting Constructor
ComputeNodalUserObjectsThread::ComputeNodalUserObjectsThread(ComputeNodalUserObjectsThread & x, Threads::split split) :
    ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _user_objects(x._user_objects),
    _group(x._group)
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

  // All Nodes
  for (std::vector<NodalUserObject *>::const_iterator nodal_user_object_it =
         _user_objects[_tid].nodalUserObjects(Moose::ANY_BOUNDARY_ID, _group).begin();
       nodal_user_object_it != _user_objects[_tid].nodalUserObjects(Moose::ANY_BOUNDARY_ID, _group).end();
       ++nodal_user_object_it)
  {
    (*nodal_user_object_it)->execute();
  }

  // Boundary Restricted UserObjects
  std::vector<BoundaryID> nodeset_ids;
  _fe_problem.mesh().getMesh().get_boundary_info().boundary_ids(node, nodeset_ids);

  for (std::vector<BoundaryID>::iterator it = nodeset_ids.begin(); it != nodeset_ids.end(); ++it)
  {
    for (std::vector<NodalUserObject *>::const_iterator nodal_user_object_it = _user_objects[_tid].nodalUserObjects(*it, _group).begin();
         nodal_user_object_it != _user_objects[_tid].nodalUserObjects(*it, _group).end();
         ++nodal_user_object_it)
    {
      (*nodal_user_object_it)->execute();
    }
  }

  // Subdomain Restricted UserObjects
  const std::set<SubdomainID> & block_ids = _fe_problem.mesh().getNodeBlockIds(*node);
  for (std::set<SubdomainID>::const_iterator block_it = block_ids.begin(); block_it != block_ids.end(); ++block_it)
  {
    for (std::vector<NodalUserObject *>::const_iterator nodal_user_object_it = _user_objects[_tid].blockNodalUserObjects(*block_it, _group).begin();
         nodal_user_object_it != _user_objects[_tid].blockNodalUserObjects(*block_it, _group).end();
         ++nodal_user_object_it)
    {
      (*nodal_user_object_it)->execute();
    }
  }
}

void
ComputeNodalUserObjectsThread::join(const ComputeNodalUserObjectsThread & /*y*/)
{
}
