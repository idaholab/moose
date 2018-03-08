//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTENODALKERNELBCJACOBIANSTHREAD_H
#define COMPUTENODALKERNELBCJACOBIANSTHREAD_H

#include "MooseMesh.h"
#include "ThreadedNodeLoop.h"

class AuxiliarySystem;
class NodalKernel;

class ComputeNodalKernelBCJacobiansThread
    : public ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>
{
public:
  ComputeNodalKernelBCJacobiansThread(FEProblemBase & fe_problem,
                                      const MooseObjectWarehouse<NodalKernel> & nodal_kernels,
                                      SparseMatrix<Number> & jacobian);

  // Splitting Constructor
  ComputeNodalKernelBCJacobiansThread(ComputeNodalKernelBCJacobiansThread & x,
                                      Threads::split split);

  virtual void pre() override;

  virtual void onNode(ConstBndNodeRange::const_iterator & node_it) override;

  void join(const ComputeNodalKernelBCJacobiansThread & /*y*/);

protected:
  AuxiliarySystem & _aux_sys;

  const MooseObjectWarehouse<NodalKernel> & _nodal_kernels;

  SparseMatrix<Number> & _jacobian;

  /// Number of contributions cached up
  unsigned int _num_cached;
};

#endif // COMPUTENODALKERNELBCJACOBIANSTHREAD_H
