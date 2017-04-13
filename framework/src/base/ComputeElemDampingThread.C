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
#include "ComputeElemDampingThread.h"
#include "NonlinearSystemBase.h"
#include "NonlinearSystem.h"
#include "Problem.h"
#include "ElementDamper.h"

// libMesh includes
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
