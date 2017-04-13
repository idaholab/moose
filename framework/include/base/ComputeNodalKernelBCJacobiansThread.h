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
