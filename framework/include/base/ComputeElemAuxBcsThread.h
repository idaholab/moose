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

#ifndef COMPUTEELEMAUXBCSTHREAD_H
#define COMPUTEELEMAUXBCSTHREAD_H

#include "ParallelUniqueId.h"
#include "AuxWarehouse.h"
#include "BndElement.h"

class FEProblem;
class AuxiliarySystem;


class ComputeElemAuxBcsThread
{
public:
  ComputeElemAuxBcsThread(FEProblem & problem, AuxiliarySystem & sys, std::vector<AuxWarehouse> & auxs);
  // Splitting Constructor
  ComputeElemAuxBcsThread(ComputeElemAuxBcsThread & x, Threads::split split);

  void operator() (const ConstBndElemRange & range);

  void join(const ComputeElemAuxBcsThread & /*y*/);

protected:
  FEProblem & _problem;
  AuxiliarySystem & _sys;
  THREAD_ID _tid;

  std::vector<AuxWarehouse> & _auxs;
};

#endif //COMPUTEELEMAUXBCSTHREAD_H
