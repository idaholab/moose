//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelUniqueId.h"
#include "FEProblemBase.h"
#include "ThreadedElementLoopBase.h"

// Forward declarations
class SystemBase;

/**
 * This mutex is used by all derived classes of the ThreadedElementLoop. It
 * is necessary to protect the creation of the strings used in the propogation
 * of the error messages.  It's possible for a thread to have acquired the
 * commonly used mutex in the Threads namespace so this one is here to
 * avoid any deadlocking.
 */
static Threads::spin_mutex threaded_element_mutex;

/**
 * Base class for assembly-like calculations.
 */
template <typename RangeType>
class ThreadedElementLoop : public ThreadedElementLoopBase<RangeType>
{
public:
  ThreadedElementLoop(FEProblemBase & feproblem);

  ThreadedElementLoop(ThreadedElementLoop & x, Threads::split split);

  virtual ~ThreadedElementLoop();

  virtual void caughtMooseException(MooseException & e) override;

  virtual bool keepGoing() override { return !_fe_problem.hasException(); }

  virtual void preElement(const Elem * elem) override;

  virtual void preInternalSide(const Elem * elem, unsigned int side) override;

  virtual void preBoundary(const Elem * elem,
                           unsigned int side,
                           BoundaryID bnd_id,
                           const Elem * lower_d_elem = nullptr) override;

  virtual void neighborSubdomainChanged() override;

protected:
  FEProblemBase & _fe_problem;
};

template <typename RangeType>
ThreadedElementLoop<RangeType>::ThreadedElementLoop(FEProblemBase & fe_problem)
  : ThreadedElementLoopBase<RangeType>(fe_problem.mesh()), _fe_problem(fe_problem)
{
}

template <typename RangeType>
ThreadedElementLoop<RangeType>::ThreadedElementLoop(ThreadedElementLoop & x,
                                                    Threads::split /*split*/)
  : ThreadedElementLoopBase<RangeType>(x), _fe_problem(x._fe_problem)
{
}

template <typename RangeType>
ThreadedElementLoop<RangeType>::~ThreadedElementLoop()
{
}

template <typename RangeType>
void
ThreadedElementLoop<RangeType>::caughtMooseException(MooseException & e)
{
  Threads::spin_mutex::scoped_lock lock(threaded_element_mutex);

  std::string what(e.what());
  _fe_problem.setException(what);
}

template <typename RangeType>
void
ThreadedElementLoop<RangeType>::preElement(const Elem * el)
{
  _fe_problem.setCurrentSubdomainID(el, ThreadedElementLoopBase<RangeType>::_tid);
}

template <typename RangeType>
void
ThreadedElementLoop<RangeType>::preInternalSide(const Elem * el, unsigned int side)
{
  _fe_problem.setNeighborSubdomainID(el, side, ThreadedElementLoopBase<RangeType>::_tid);
}

template <typename RangeType>
void
ThreadedElementLoop<RangeType>::preBoundary(const Elem * /*elem*/,
                                            unsigned int /*side*/,
                                            BoundaryID bnd_id,
                                            const Elem * /*=nullptr*/)
{
  _fe_problem.setCurrentBoundaryID(bnd_id, ThreadedElementLoopBase<RangeType>::_tid);
}

template <typename RangeType>
void
ThreadedElementLoop<RangeType>::neighborSubdomainChanged()
{
  _fe_problem.neighborSubdomainSetup(ThreadedElementLoopBase<RangeType>::_neighbor_subdomain,
                                     ThreadedElementLoopBase<RangeType>::_tid);
}
