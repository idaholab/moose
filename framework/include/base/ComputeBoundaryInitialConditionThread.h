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

#ifndef COMPUTEREBOUNDARYINITIALCONDITIONTHREAD_H
#define COMPUTEREBOUNDARYINITIALCONDITIONTHREAD_H

#include "Moose.h"
#include "ParallelUniqueId.h"
#include "BndNode.h"
// libmesh
#include "libmesh/elem_range.h"
#include "libmesh/numeric_vector.h"

class FEProblem;

class ComputeBoundaryInitialConditionThread
{
public:
  ComputeBoundaryInitialConditionThread(FEProblem & fe_problem);
  // Splitting Constructor
  ComputeBoundaryInitialConditionThread(ComputeBoundaryInitialConditionThread & x, Threads::split split);

  void operator() (const ConstBndNodeRange & range);
  void join(const ComputeBoundaryInitialConditionThread & /*y*/);

protected:
  FEProblem & _fe_problem;
  THREAD_ID _tid;
};

#endif //COMPUTEBOUNDARYINITIALCONDITIONTHREAD_H
