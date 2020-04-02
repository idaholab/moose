//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ComputeNodalDampingThread.h"
#include "NonlinearSystem.h"
#include "Problem.h"
#include "NodalDamper.h"

#include "libmesh/threads.h"

ComputeNodalDampingThread::ComputeNodalDampingThread(FEProblemBase & feproblem)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(feproblem),
    _damping(1.0),
    _nl(feproblem.getNonlinearSystemBase()),
    _nodal_dampers(_nl.getNodalDamperWarehouse())
{
}

// Splitting Constructor
ComputeNodalDampingThread::ComputeNodalDampingThread(ComputeNodalDampingThread & x,
                                                     Threads::split split)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _damping(1.0),
    _nl(x._nl),
    _nodal_dampers(x._nodal_dampers)
{
}

ComputeNodalDampingThread::~ComputeNodalDampingThread() {}

void
ComputeNodalDampingThread::onNode(ConstNodeRange::const_iterator & node_it)
{
  const Node * node = *node_it;
  _fe_problem.reinitNode(node, _tid);

  std::set<MooseVariable *> damped_vars;
  const auto & node_subdomains = _fe_problem.mesh().getNodeBlockIds(*node);

  const auto & ndampers = _nl.getNodalDamperWarehouse().getActiveObjects(_tid);
  for (const auto & damper : ndampers)
  {
    auto damped_var = damper->getVariable();
    auto damped_var_subdomains = damped_var->activeSubdomains();
    if (damped_var_subdomains.empty())
      damped_vars.insert(damped_var);
    else
    {
      std::set<SubdomainID> intersect;
      std::set_intersection(damped_var_subdomains.begin(),
                            damped_var_subdomains.end(),
                            node_subdomains.begin(),
                            node_subdomains.end(),
                            std::inserter(intersect, intersect.end()));
      if (!intersect.empty())
        damped_vars.insert(damped_var);
    }
  }

  if (!damped_vars.empty())
  {
    _nl.reinitIncrementAtNodeForDampers(_tid, damped_vars);

    const auto & objects = _nodal_dampers.getActiveObjects(_tid);
    for (const auto & obj : objects)
    {
      Real cur_damping = obj->computeDamping();
      obj->checkMinDamping(cur_damping);
      if (cur_damping < _damping)
        _damping = cur_damping;
    }
  }
}

Real
ComputeNodalDampingThread::damping()
{
  return _damping;
}

void
ComputeNodalDampingThread::join(const ComputeNodalDampingThread & y)
{
  if (y._damping < _damping)
    _damping = y._damping;
}
