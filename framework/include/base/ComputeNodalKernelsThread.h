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

#ifndef COMPUTENODALKERNELSTHREAD_H
#define COMPUTENODALKERNELSTHREAD_H

#include "ParallelUniqueId.h"
#include "NodalKernelWarehouse.h"

// libMesh includes
#include "libmesh/node_range.h"
#include "libmesh/numeric_vector.h"


class FEProblem;
class AuxiliarySystem;


class ComputeNodalKernelsThread
{
public:
  ComputeNodalKernelsThread(FEProblem & fe_problem, AuxiliarySystem & sys, std::vector<NodalKernelWarehouse> & nodal_kernels);

  // Splitting Constructor
  ComputeNodalKernelsThread(ComputeNodalKernelsThread & x, Threads::split split);

  void operator() (const ConstNodeRange & range);

  void join(const ComputeNodalKernelsThread & /*y*/);

protected:
  FEProblem & _fe_problem;
  AuxiliarySystem & _sys;
  THREAD_ID _tid;

  std::vector<NodalKernelWarehouse> & _nodal_kernels;
};

#endif //COMPUTENODALKERNELSTHREAD_H
