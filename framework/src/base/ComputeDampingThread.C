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

// libmesh includes
#include "threads.h"

ComputeDampingThread::ComputeDampingThread(Problem & problem,
                                           NonlinearSystem & sys) :
    ThreadedElementLoop<ConstElemRange>(problem, sys),
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

void
ComputeDampingThread::onElement(const Elem *elem)
{
  _problem.prepare(elem, _tid);
  _problem.reinitElem(elem, _tid);
  _nl.reinitDampers(_tid);

  DamperIterator damper_begin = _nl._dampers[_tid].dampersBegin();
  DamperIterator damper_end = _nl._dampers[_tid].dampersEnd();
  DamperIterator damper_it = damper_begin;

  for(damper_it=damper_begin;damper_it!=damper_end;++damper_it)
  {
    Real cur_damping = (*damper_it)->computeDamping();
    if(cur_damping < _damping)
      _damping = cur_damping;
  }
}

void
ComputeDampingThread::join(const ComputeDampingThread & y)
{
  if(y._damping < _damping)
    _damping = y._damping;
}
