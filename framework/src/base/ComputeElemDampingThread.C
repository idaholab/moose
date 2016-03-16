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
#include "NonlinearSystem.h"
#include "Problem.h"
#include "ElementDamper.h"

// libMesh includes
#include "libmesh/threads.h"

ComputeElemDampingThread::ComputeElemDampingThread(FEProblem & feproblem,
                                                   NonlinearSystem & sys) :
    ThreadedElementLoop<ConstElemRange>(feproblem, sys),
    _damping(1.0),
    _nl(sys),
    _element_dampers(sys.getElementDamperWarehouse())
{
}

// Splitting Constructor
ComputeElemDampingThread::ComputeElemDampingThread(ComputeElemDampingThread & x, Threads::split split) :
    ThreadedElementLoop<ConstElemRange>(x, split),
    _damping(1.0),
    _nl(x._nl),
    _element_dampers(x._element_dampers)
{
}

ComputeElemDampingThread::~ComputeElemDampingThread()
{
}

void
ComputeElemDampingThread::onElement(const Elem *elem)
{
  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);
  _nl.reinitIncrementForDampers(_tid);

  const std::vector<MooseSharedPointer<ElementDamper> > & objects = _element_dampers.getActiveObjects(_tid);
  for (std::vector<MooseSharedPointer<ElementDamper> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
  {
    Real cur_damping = (*it)->computeDamping();
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
