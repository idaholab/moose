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

#ifndef MAXQPSTHREAD_H
#define MAXQPSTHREAD_H

#include "Moose.h"
#include "ParallelUniqueId.h"

// libMesh includes
#include "libmesh/node_range.h"
#include "libmesh/system.h"

class FEProblem;

/**
 * Grab all the local dof indices for the variables passed in, in the system passed in.
 */
class MaxQpsThread
{
public:
  MaxQpsThread(FEProblem & fe_problem);

  // Splitting Constructor
  MaxQpsThread(MaxQpsThread & x, Threads::split split);

  void operator() (const ConstElemRange & range);

  void join(const MaxQpsThread & y);

  unsigned int max() { return _max; }

protected:
  FEProblem & _fe_problem;

  THREAD_ID _tid;

  /// Maximum number of qps encountered
  unsigned int _max;
};

#endif //MAXQPSTHREAD_H
