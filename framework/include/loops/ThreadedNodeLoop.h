//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblemBase.h"
#include "ParallelUniqueId.h"

static Threads::spin_mutex threaded_node_mutex;

/**
 * Template for a threaded (e.g. replicated on each thread here) loop on a range of nodes
 * @tparam RangeType the class type of the range
 * @tparam IteratorType the type of the iteration
 * @tparam Derived the type of the Derived class. This is for CRTP
 * See https://en.cppreference.com/w/cpp/language/crtp.html
 */
template <typename RangeType, typename IteratorType, typename Derived>
class ThreadedNodeLoop
{
public:
  ThreadedNodeLoop(FEProblemBase & fe_problem);

  // Splitting Constructor
  ThreadedNodeLoop(ThreadedNodeLoop & x, Threads::split split);

  ~ThreadedNodeLoop() {};

  void operator()(const RangeType & range);

  /**
   * Called before the node range loop
   */
  void pre();

  /**
   * Called after the node range loop
   */
  void post();

  /**
   * Called for each node
   */
  void onNode(IteratorType & node_it);

  /**
   * Called after the node assembly is done (including surface assembling)
   *
   * @param node - active node
   */
  void postNode(IteratorType & node_it);

  /**
   * Called if a MooseException is caught anywhere during the computation.
   * The single input parameter taken is a MooseException object.
   */
  void caughtMooseException(MooseException & e)
  {
    Threads::spin_mutex::scoped_lock lock(threaded_node_mutex);

    std::string what(e.what());
    _fe_problem.setException(what);
  };

  /**
   * Whether or not the loop should continue.
   *
   * @return true to keep going, false to stop.
   */
  bool keepGoing() { return !_fe_problem.hasException(); }

protected:
  /// The Problem class, stores objects, knows the current execution point, etc
  FEProblemBase & _fe_problem;
  /// Thread ID. This loop is replicated on each thread
  THREAD_ID _tid;

  /// Print information about the loop, mostly order of execution of objects
  void printGeneralExecutionInformation() const {}
};

template <typename RangeType, typename IteratorType, typename Derived>
ThreadedNodeLoop<RangeType, IteratorType, Derived>::ThreadedNodeLoop(FEProblemBase & fe_problem)
  : _fe_problem(fe_problem)
{
}

template <typename RangeType, typename IteratorType, typename Derived>
ThreadedNodeLoop<RangeType, IteratorType, Derived>::ThreadedNodeLoop(ThreadedNodeLoop & x,
                                                                     Threads::split /*split*/)
  : _fe_problem(x._fe_problem)
{
}

template <typename RangeType, typename IteratorType, typename Derived>
void
ThreadedNodeLoop<RangeType, IteratorType, Derived>::operator()(const RangeType & range)
{
  try
  {
    ParallelUniqueId puid;
    _tid = puid.id;

    static_cast<Derived *>(this)->pre();
    static_cast<Derived *>(this)->printGeneralExecutionInformation();

    for (IteratorType nd = range.begin(); nd != range.end(); ++nd)
    {
      if (!static_cast<Derived *>(this)->keepGoing())
        break;

      static_cast<Derived *>(this)->onNode(nd);

      static_cast<Derived *>(this)->postNode(nd);
    }

    static_cast<Derived *>(this)->post();
  }
  catch (MooseException & e)
  {
    static_cast<Derived *>(this)->caughtMooseException(e);
  }
}

template <typename RangeType, typename IteratorType, typename Derived>
void
ThreadedNodeLoop<RangeType, IteratorType, Derived>::pre()
{
}

template <typename RangeType, typename IteratorType, typename Derived>
void
ThreadedNodeLoop<RangeType, IteratorType, Derived>::post()
{
}

template <typename RangeType, typename IteratorType, typename Derived>
void
ThreadedNodeLoop<RangeType, IteratorType, Derived>::onNode(IteratorType & /*node_it*/)
{
}

template <typename RangeType, typename IteratorType, typename Derived>
void
ThreadedNodeLoop<RangeType, IteratorType, Derived>::postNode(IteratorType & /*node_it*/)
{
}
