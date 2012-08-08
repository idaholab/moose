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

#ifndef COMPUTEREINITIALCONDITIONTHREAD_H
#define COMPUTEREINITIALCONDITIONTHREAD_H

#include "ParallelUniqueId.h"
#include "elem_range.h"
#include "numeric_vector.h"

class SubProblem;
class SystemBase;

class ComputeInitialConditionThread
{
public:
  ComputeInitialConditionThread(SubProblem & subproblem, SystemBase & sys, NumericVector<Number> & solution);
  // Splitting Constructor
  ComputeInitialConditionThread(ComputeInitialConditionThread & x, Threads::split split);

  void operator() (const ConstElemRange & range);
  void join(const ComputeInitialConditionThread & /*y*/);

protected:
  SubProblem & _subproblem;
  SystemBase & _sys;
  THREAD_ID _tid;
  NumericVector<Number> & _solution;
};

#endif //COMPUTEINITIALCONDITIONTHREAD_H
