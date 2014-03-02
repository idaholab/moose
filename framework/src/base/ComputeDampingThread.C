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

#include "ComputeDampingThread.h"
#include "NonlinearSystem.h"
#include "Problem.h"
#include "Damper.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeDampingThread::ComputeDampingThread(FEProblem & feproblem,
                                           NonlinearSystem & sys) :
    ThreadedElementLoop<ConstElemRange>(feproblem, sys),
    _damping(1.0),
    _nl(sys)
{
}

// Splitting Constructor
ComputeDampingThread::ComputeDampingThread(ComputeDampingThread & x, Threads::split split) :
    ThreadedElementLoop<ConstElemRange>(x, split),
    _damping(1.0),
    _nl(x._nl)
{
}

ComputeDampingThread::~ComputeDampingThread()
{
}

void
ComputeDampingThread::onElement(const Elem *elem)
{
  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);
  _nl.reinitDampers(_tid);

  for(std::vector<Damper *>::const_iterator damper_it = _nl._dampers[_tid].all().begin();
      damper_it != _nl._dampers[_tid].all().end();
      ++damper_it)
  {
    Real cur_damping = (*damper_it)->computeDamping();
    if (cur_damping < _damping)
      _damping = cur_damping;
  }
}

Real
ComputeDampingThread::damping()
{
  return _damping;
}

void
ComputeDampingThread::join(const ComputeDampingThread & y)
{
  if (y._damping < _damping)
    _damping = y._damping;
}
