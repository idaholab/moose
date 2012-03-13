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

#ifndef COMPUTENODALPPSTHREAD_H
#define COMPUTENODALPPSTHREAD_H

#include "ParallelUniqueId.h"
#include "PostprocessorWarehouse.h"
// libMesh includes
#include "node_range.h"

class Problem;

class ComputeNodalPPSThread
{
public:
  ComputeNodalPPSThread(Problem & problem, std::vector<PostprocessorWarehouse> & pps);
  // Splitting Constructor
  ComputeNodalPPSThread(ComputeNodalPPSThread & x, Threads::split split);

  void operator() (const ConstNodeRange & range);

  void join(const ComputeNodalPPSThread & /*y*/);

protected:
  Problem & _problem;
  THREAD_ID _tid;

  std::vector<PostprocessorWarehouse> & _pps;
};

#endif //COMPUTENODALPPSTHREAD_H
