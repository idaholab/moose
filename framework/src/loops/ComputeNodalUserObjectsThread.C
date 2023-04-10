//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeNodalUserObjectsThread.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseMesh.h"
#include "NodalUserObject.h"
#include "AuxiliarySystem.h"

#include "libmesh/threads.h"

Threads::spin_mutex ComputeNodalUserObjectsThread::writable_variable_mutex;

ComputeNodalUserObjectsThread::ComputeNodalUserObjectsThread(FEProblemBase & fe_problem,
                                                             const TheWarehouse::Query & query)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(fe_problem),
    _query(query),
    _aux_sys(fe_problem.getAuxiliarySystem())
{
}

// Splitting Constructor
ComputeNodalUserObjectsThread::ComputeNodalUserObjectsThread(ComputeNodalUserObjectsThread & x,
                                                             Threads::split split)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _query(x._query),
    _aux_sys(x._aux_sys)
{
}

ComputeNodalUserObjectsThread::~ComputeNodalUserObjectsThread() {}

void
ComputeNodalUserObjectsThread::onNode(ConstNodeRange::const_iterator & node_it)
{
  const Node * node = *node_it;
  _fe_problem.reinitNode(node, _tid);

  std::vector<NodalUserObject *> objs;

  // Boundary Restricted
  std::vector<BoundaryID> nodeset_ids;
  _fe_problem.mesh().getMesh().get_boundary_info().boundary_ids(node, nodeset_ids);
  for (const auto & bnd : nodeset_ids)
  {
    _query.clone()
        .condition<AttribThread>(_tid)
        .condition<AttribInterfaces>(Interfaces::NodalUserObject)
        .condition<AttribBoundaries>(bnd, true)
        .queryInto(objs);
    for (const auto & uo : objs)
    {
      uo->execute();

      // update the aux solution vector if writable coupled variables are used
      if (uo->hasWritableCoupledVariables())
      {
        Threads::spin_mutex::scoped_lock lock(writable_variable_mutex);
        for (auto * var : uo->getWritableCoupledVariables())
          var->insert(_aux_sys.solution());
      }
    }
  }

  // Block Restricted
  // NodalUserObjects may be block restricted, in this case by default the execute() method is
  // called for each subdomain that the node "belongs". This may be disabled in the NodalUserObject
  // by setting "unique_node_execute = true".

  // To enforce the unique execution this vector is populated and checked if the unique flag is
  // enabled.
  std::set<NodalUserObject *> computed;
  const std::set<SubdomainID> & block_ids = _fe_problem.mesh().getNodeBlockIds(*node);
  for (const auto & block : block_ids)
  {
    _query.clone()
        .condition<AttribThread>(_tid)
        .condition<AttribInterfaces>(Interfaces::NodalUserObject)
        .condition<AttribSubdomains>(block)
        .queryInto(objs);

    for (const auto & uo : objs)
      if (!uo->isUniqueNodeExecute() || computed.count(uo) == 0)
      {
        uo->execute();

        // update the aux solution vector if writable coupled variables are used
        if (uo->hasWritableCoupledVariables())
        {
          Threads::spin_mutex::scoped_lock lock(writable_variable_mutex);
          for (auto * var : uo->getWritableCoupledVariables())
            var->insert(_aux_sys.solution());
        }

        computed.insert(uo);
      }
  }
}

void
ComputeNodalUserObjectsThread::join(const ComputeNodalUserObjectsThread & /*y*/)
{
}

void
ComputeNodalUserObjectsThread::printGeneralExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid))
    return;

  // Get all nodal UOs
  std::vector<MooseObject *> nodal_uos;
  _query.clone()
      .condition<AttribThread>(_tid)
      .condition<AttribInterfaces>(Interfaces::NodalUserObject)
      .queryInto(nodal_uos);

  if (nodal_uos.size())
  {
    const auto & console = _fe_problem.console();
    const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
    console << "[DBG] Computing nodal user objects on " << execute_on << std::endl;
    mooseDoOnce(
        console << "[DBG] Ordering on nodes:" << std::endl;
        console << "[DBG] - boundary restricted user objects" << std::endl;
        console << "[DBG] - block restricted user objects" << std::endl;
        console << "[DBG] Nodal UOs executed on each node will differ based on these restrictions"
                << std::endl;);

    auto message = ConsoleUtils::mooseObjectVectorToString(nodal_uos);
    message = "Order of execution:\n" + message;
    console << ConsoleUtils::formatString(message, "[DBG]") << std::endl;
  }
}
