//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ComputeElemDampingThread.h"
#include "NonlinearSystemBase.h"
#include "NonlinearSystem.h"
#include "Problem.h"
#include "ElementDamper.h"

#include "libmesh/threads.h"

ComputeElemDampingThread::ComputeElemDampingThread(FEProblemBase & feproblem)
  : ThreadedElementLoop<ConstElemRange>(feproblem),
    _damping(1.0),
    _nl(feproblem.getNonlinearSystemBase()),
    _element_dampers(_nl.getElementDamperWarehouse())
{
}

// Splitting Constructor
ComputeElemDampingThread::ComputeElemDampingThread(ComputeElemDampingThread & x,
                                                   Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _damping(1.0),
    _nl(x._nl),
    _element_dampers(x._element_dampers)
{
}

ComputeElemDampingThread::~ComputeElemDampingThread() {}

void
ComputeElemDampingThread::onElement(const Elem * elem)
{
  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);

  std::set<MooseVariable *> damped_vars;

  const std::vector<std::shared_ptr<ElementDamper>> & edampers =
      _nl.getElementDamperWarehouse().getActiveObjects(_tid);
  for (const auto & damper : edampers)
    damped_vars.insert(damper->getVariable());

  _nl.reinitIncrementAtQpsForDampers(_tid, damped_vars);

  const std::vector<std::shared_ptr<ElementDamper>> & objects =
      _element_dampers.getActiveObjects(_tid);
  for (const auto & obj : objects)
  {
    Real cur_damping = obj->computeDamping();
    obj->checkMinDamping(cur_damping);
    if (cur_damping < _damping)
      _damping = cur_damping;
  }
}

Real
ComputeElemDampingThread::damping()
{
  return _damping;
}

void
ComputeElemDampingThread::join(const ComputeElemDampingThread & y)
{
  if (y._damping < _damping)
    _damping = y._damping;
}

void
ComputeElemDampingThread::printGeneralExecutionInformation() const
{
  const auto & damper_wh = _nl.getElementDamperWarehouse();
  if (!_fe_problem.shouldPrintExecution(_tid) || !damper_wh.hasActiveObjects())
    return;

  const auto & console = _fe_problem.console();
  const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
  console << "[DBG] Beginning elemental loop to compute damping on " << execute_on << std::endl;
  // Dampers are currently not block restricted
  console << "[DBG] Ordering of dampers " << std::endl;
  console << damper_wh.activeObjectsToFormattedString() << std::endl;
}
