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

#ifndef COMPUTENODALKERNELJACOBIANSTHREAD_H
#define COMPUTENODALKERNELJACOBIANSTHREAD_H

#include "ParallelUniqueId.h"
#include "NodalKernelWarehouse.h"

// libMesh includes
#include "libmesh/node_range.h"
#include "libmesh/numeric_vector.h"


class FEProblem;
class AuxiliarySystem;


class ComputeNodalKernelJacobiansThread
{
public:
  ComputeNodalKernelJacobiansThread(FEProblem & fe_problem, AuxiliarySystem & sys, std::vector<NodalKernelWarehouse> & nodal_kernels,  SparseMatrix<Number> & jacobian);

  // Splitting Constructor
  ComputeNodalKernelJacobiansThread(ComputeNodalKernelJacobiansThread & x, Threads::split split);

  void operator() (const ConstNodeRange & range);

  void join(const ComputeNodalKernelJacobiansThread & /*y*/);

protected:
  FEProblem & _fe_problem;
  AuxiliarySystem & _sys;
  THREAD_ID _tid;

  std::vector<NodalKernelWarehouse> & _nodal_kernels;

  SparseMatrix<Number> & _jacobian;
};

#endif //COMPUTENODALKERNELJACOBIANSTHREAD_H
