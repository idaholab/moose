//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

ComputeNodalDampingThread::ComputeNodalDampingThread(FEProblemBase & feproblem,
                                                     NonlinearSystemBase & nl)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(feproblem),
    _damping(1.0),
    _nl(nl),
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

  const auto & ndampers = _nodal_dampers.getActiveObjects(_tid);
  for (const auto & damper : ndampers)
    if (damper->variableDefinedOnNode(node))
      damped_vars.insert(damper->getVariable());

  if (!damped_vars.empty())
    _nl.reinitIncrementAtNodeForDampers(_tid, damped_vars);

  for (const auto & damper : ndampers)
  {
    if (!damper->variableDefinedOnNode(node))
      continue;

    Real cur_damping = damper->computeDamping();
    damper->checkMinDamping(cur_damping);
    if (cur_damping < _damping)
      _damping = cur_damping;
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

void
ComputeNodalDampingThread::printGeneralExecutionInformation() const
{
  const auto & damper_wh = _nl.getNodalDamperWarehouse();
  if (!_fe_problem.shouldPrintExecution(_tid) || !damper_wh.hasActiveObjects())
    return;

  const auto & console = _fe_problem.console();
  const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
  console << "[DBG] Executing nodal dampers on " << execute_on << std::endl;
  console << "[DBG] Ordering of the dampers on the blocks they are defined on:" << std::endl;
  // TODO Check that all objects are active at this point
  console << damper_wh.activeObjectsToFormattedString() << std::endl;
}
