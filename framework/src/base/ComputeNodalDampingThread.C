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
#include "ComputeNodalDampingThread.h"
#include "NonlinearSystem.h"
#include "Problem.h"
#include "NodalDamper.h"

// libMesh includes
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

  const auto & ndampers = _nl.getNodalDamperWarehouse().getActiveObjects(_tid);
  for (const auto & damper : ndampers)
    damped_vars.insert(damper->getVariable());

  _nl.reinitIncrementAtNodeForDampers(_tid, damped_vars);

  const auto & objects = _nodal_dampers.getActiveObjects(_tid);
  for (const auto & obj : objects)
  {
    Real cur_damping = obj->computeDamping();
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
