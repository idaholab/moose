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
#include "SubProblem.h"
#include "NodalUserObject.h"

// libmesh includes
#include "threads.h"

ComputeNodalUserObjectsThread::ComputeNodalUserObjectsThread(SubProblem & problem, std::vector<UserObjectWarehouse> & user_objects, UserObjectWarehouse::GROUP group) :
    _sub_problem(problem),
    _user_objects(user_objects),
    _group(group)
{
}

// Splitting Constructor
ComputeNodalUserObjectsThread::ComputeNodalUserObjectsThread(ComputeNodalUserObjectsThread & x, Threads::split /*split*/) :
    _sub_problem(x._sub_problem),
    _user_objects(x._user_objects),
    _group(x._group)
{
}

ComputeNodalUserObjectsThread::~ComputeNodalUserObjectsThread()
{
}

void
ComputeNodalUserObjectsThread::operator() (const ConstNodeRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  for (ConstNodeRange::const_iterator node_it = range.begin() ; node_it != range.end(); ++node_it)
  {
    const Node * node = *node_it;
    _sub_problem.reinitNode(node, _tid);

    // All Nodes
    for (std::vector<NodalUserObject *>::const_iterator nodal_user_object_it =
           _user_objects[_tid].nodalUserObjects(Moose::ANY_BOUNDARY_ID, _group).begin();
         nodal_user_object_it != _user_objects[_tid].nodalUserObjects(Moose::ANY_BOUNDARY_ID, _group).end();
         ++nodal_user_object_it)
    {
      (*nodal_user_object_it)->execute();
    }

    // Boundary Restricted UserObjects
    std::vector<BoundaryID> nodeset_ids = _sub_problem.mesh().getMesh().boundary_info->boundary_ids(node);

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
    const std::set<SubdomainID> & block_ids = _sub_problem.mesh().getNodeBlockIds(*node);
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
}

void
ComputeNodalUserObjectsThread::join(const ComputeNodalUserObjectsThread & /*y*/)
{
}
