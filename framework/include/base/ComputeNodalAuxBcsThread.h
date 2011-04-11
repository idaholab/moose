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

#ifndef COMPUTENODALAUXBCSTHREAD_H
#define COMPUTENODALAUXBCSTHREAD_H

#include "ParallelUniqueId.h"
#include "AuxWarehouse.h"
#include "BndNode.h"

class Problem;
class AuxiliarySystem;


class ComputeNodalAuxBcsThread
{
public:
  ComputeNodalAuxBcsThread(Problem & problem, AuxiliarySystem & sys, std::vector<AuxWarehouse> & auxs);
  // Splitting Constructor
  ComputeNodalAuxBcsThread(ComputeNodalAuxBcsThread & x, Threads::split split);

  void operator() (const ConstBndNodeRange & range);

  void join(const ComputeNodalAuxBcsThread & /*y*/);

protected:
  Problem & _problem;
  AuxiliarySystem & _sys;
  THREAD_ID _tid;

  std::vector<AuxWarehouse> & _auxs;
};

#endif //COMPUTENODALAUXBCSTHREAD_H
