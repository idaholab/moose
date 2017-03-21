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

#include "ThreadedNodeLoop.h"

// Forward declarations
class FEProblemBase;

class ComputeBoundaryInitialConditionThread
    : public ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>
{
public:
  ComputeBoundaryInitialConditionThread(FEProblemBase & fe_problem);

  // Splitting Constructor
  ComputeBoundaryInitialConditionThread(ComputeBoundaryInitialConditionThread & x,
                                        Threads::split split);

  void onNode(ConstBndNodeRange::const_iterator & nd);

  void join(const ComputeBoundaryInitialConditionThread & /*y*/);
};

#endif // COMPUTEBOUNDARYINITIALCONDITIONTHREAD_H
