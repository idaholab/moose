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

#ifndef COMPUTEELEMAUXVARSTHREAD_H
#define COMPUTEELEMAUXVARSTHREAD_H

#include "ParallelUniqueId.h"
#include "AuxWarehouse.h"
// libMesh includes
#include "node_range.h"

class FEProblem;
class AuxiliarySystem;


class ComputeElemAuxVarsThread
{
public:
  ComputeElemAuxVarsThread(FEProblem & mproblem, AuxiliarySystem & sys, std::vector<AuxWarehouse> & auxs);
  // Splitting Constructor
  ComputeElemAuxVarsThread(ComputeElemAuxVarsThread & x, Threads::split split);

  void operator() (const ConstElemRange & range);

  void join(const ComputeElemAuxVarsThread & /*y*/);

protected:
  FEProblem & _mproblem;
  AuxiliarySystem & _sys;
  THREAD_ID _tid;

  std::vector<AuxWarehouse> & _auxs;
};

#endif //COMPUTEELEMAUXVARSTHREAD_H
