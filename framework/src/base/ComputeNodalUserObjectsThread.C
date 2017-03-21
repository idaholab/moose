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

ComputeNodalUserObjectsThread::ComputeNodalUserObjectsThread(
    FEProblemBase & fe_problem, const MooseObjectWarehouse<NodalUserObject> & user_objects)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(fe_problem),
    _user_objects(user_objects)
{
}

// Splitting Constructor
ComputeNodalUserObjectsThread::ComputeNodalUserObjectsThread(ComputeNodalUserObjectsThread & x,
                                                             Threads::split split)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _user_objects(x._user_objects)
{
}

ComputeNodalUserObjectsThread::~ComputeNodalUserObjectsThread() {}

void
ComputeNodalUserObjectsThread::onNode(ConstNodeRange::const_iterator & node_it)
{
  const Node * node = *node_it;
  _fe_problem.reinitNode(node, _tid);

  // Boundary Restricted
  std::vector<BoundaryID> nodeset_ids;
  _fe_problem.mesh().getMesh().get_boundary_info().boundary_ids(node, nodeset_ids);
  for (const auto & bnd : nodeset_ids)
  {
    if (_user_objects.hasActiveBoundaryObjects(bnd, _tid))
    {
      const auto & objects = _user_objects.getActiveBoundaryObjects(bnd, _tid);
      for (const auto & uo : objects)
        uo->execute();
    }
  }

  // Block Restricted
  // NodalUserObjects may be block restricted, in this case by default the execute() method is
  // called for
  // each subdomain that the node "belongs". This may be disabled in the NodalUserObject by setting
  // "unique_node_execute = true".

  // To inforce the unique execution this vector is populated and checked if the unique flag is
  // enabled.
  std::vector<std::shared_ptr<NodalUserObject>> computed;

  const std::set<SubdomainID> & block_ids = _fe_problem.mesh().getNodeBlockIds(*node);
  for (const auto & block : block_ids)
    if (_user_objects.hasActiveBlockObjects(block, _tid))
    {
      const auto & objects = _user_objects.getActiveBlockObjects(block, _tid);
      for (const auto & uo : objects)
        if (!uo->isUniqueNodeExecute() || std::count(computed.begin(), computed.end(), uo) == 0)
        {
          uo->execute();
          computed.push_back(uo);
        }
    }
}

void
ComputeNodalUserObjectsThread::join(const ComputeNodalUserObjectsThread & /*y*/)
{
}
