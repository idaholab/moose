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
#include "MooseTypes.h"
#include "MooseException.h"
#include "GeneralUserObject.h"

typedef StoredRange<std::vector<GeneralUserObject *>::iterator, GeneralUserObject *>
    GeneralUserObjectRange;

/**
 * This mutex is used to protect the creation of the strings used in the propogation
 * of the error messages.  It's possible for a thread to have acquired the
 * commonly used mutex in the Threads namespace so this one is here to
 * avoid any deadlocking.
 */
static Threads::spin_mutex threaded_general_user_objects_mutex;

/**
 * Thread to compute threaded general user objects
 *
 * This works on a range of thread IDs, so that we always compute all user objects for every
 * threaded copy.
 */
class ComputeThreadedGeneralUserObjectsThread
{
public:
  ComputeThreadedGeneralUserObjectsThread(FEProblemBase & fe_problem);
  ComputeThreadedGeneralUserObjectsThread(ComputeThreadedGeneralUserObjectsThread & x,
                                          Threads::split split);

  virtual ~ComputeThreadedGeneralUserObjectsThread();

  void operator()(const GeneralUserObjectRange & range);

  void join(const ComputeThreadedGeneralUserObjectsThread & /*y*/) {}

  /**
   * Called if a MooseException is caught anywhere during the computation.
   * The single input parameter taken is a MooseException object.
   */
  virtual void caughtMooseException(MooseException &);

protected:
  /// Print information about the loop, mostly order of execution of objects
  void printGeneralExecutionInformation(const GeneralUserObjectRange & range) const;

  /// FEProblem running this thread
  FEProblemBase & _fe_problem;
};
