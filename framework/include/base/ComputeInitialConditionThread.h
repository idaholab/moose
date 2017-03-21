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

#include "MooseTypes.h"

// libmesh
#include "libmesh/elem_range.h"
#include "libmesh/threads.h"

class FEProblemBase;

class ComputeInitialConditionThread
{
public:
  ComputeInitialConditionThread(FEProblemBase & fe_problem);
  // Splitting Constructor
  ComputeInitialConditionThread(ComputeInitialConditionThread & x, Threads::split split);

  void operator()(const ConstElemRange & range);
  void join(const ComputeInitialConditionThread & /*y*/);

protected:
  FEProblemBase & _fe_problem;
  THREAD_ID _tid;
};

#endif // COMPUTEINITIALCONDITIONTHREAD_H
