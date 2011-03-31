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

#ifndef COMPUTENODALAUXVARSTHREAD_H
#define COMPUTENODALAUXVARSTHREAD_H

#include "ParallelUniqueId.h"
#include "AuxWarehouse.h"
// libMesh includes
#include "node_range.h"

class Problem;
class AuxiliarySystem;


class ComputeNodalAuxVarsThread
{
public:
  ComputeNodalAuxVarsThread(Problem & problem, AuxiliarySystem & sys, std::vector<AuxWarehouse> & auxs);
  // Splitting Constructor
  ComputeNodalAuxVarsThread(ComputeNodalAuxVarsThread & x, Threads::split split);

  void operator() (const ConstNodeRange & range);

  void join(const ComputeNodalAuxVarsThread & /*y*/);

protected:
  Problem & _problem;
  AuxiliarySystem & _sys;
  THREAD_ID _tid;

  std::vector<AuxWarehouse> & _auxs;
};

#endif //COMPUTENODALAUXVARSTHREAD_H
