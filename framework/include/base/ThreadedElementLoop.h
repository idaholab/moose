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

#ifndef THREADEDELEMENTLOOP_H
#define THREADEDELEMENTLOOP_H

#include "ParallelUniqueId.h"
#include "MooseMesh.h"
#include "FEProblem.h"
#include "SystemBase.h"
#include "ThreadedElementLoopBase.h"

/**
 * Base class for assembling-like calculations
 *
 */
template<typename RangeType>
class ThreadedElementLoop : public ThreadedElementLoopBase<RangeType>
{
public:
  ThreadedElementLoop(FEProblem & feproblem, SystemBase & system);

  ThreadedElementLoop(ThreadedElementLoop & x, Threads::split split);

  virtual ~ThreadedElementLoop();

  virtual void caughtMooseException(MooseException & e) { std::string what(e.what()); _fe_problem.setException(what); };

  virtual bool keepGoing() { return !_fe_problem.hasException(); }
protected:
  SystemBase & _system;
  FEProblem & _fe_problem;
};


template<typename RangeType>
ThreadedElementLoop<RangeType>::ThreadedElementLoop(FEProblem & fe_problem, SystemBase & system) :
    ThreadedElementLoopBase<RangeType>(system.mesh()),
    _system(system),
    _fe_problem(fe_problem)
{
}

template<typename RangeType>
ThreadedElementLoop<RangeType>::ThreadedElementLoop(ThreadedElementLoop & x, Threads::split /*split*/) :
    ThreadedElementLoopBase<RangeType>(x),
    _system(x._system),
    _fe_problem(x._fe_problem)
{
}

template<typename RangeType>
ThreadedElementLoop<RangeType>::~ThreadedElementLoop()
{
}

#endif //THREADEDELEMENTLOOP_H
