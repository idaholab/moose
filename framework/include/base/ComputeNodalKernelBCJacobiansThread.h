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

#include "NodalKernelWarehouse.h"
#include "MooseMesh.h"
#include "ThreadedNodeLoop.h"

// libMesh includes
#include "libmesh/node_range.h"
#include "libmesh/numeric_vector.h"

class AuxiliarySystem;

class ComputeNodalKernelBCJacobiansThread : public ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>
{
public:
  ComputeNodalKernelBCJacobiansThread(FEProblem & fe_problem, AuxiliarySystem & sys, std::vector<NodalKernelWarehouse> & nodal_kernels,  SparseMatrix<Number> & jacobian);

  // Splitting Constructor
  ComputeNodalKernelBCJacobiansThread(ComputeNodalKernelBCJacobiansThread & x, Threads::split split);

  virtual void pre();

  virtual void onNode(ConstBndNodeRange::const_iterator & node_it);

  void join(const ComputeNodalKernelBCJacobiansThread & /*y*/);

protected:
  AuxiliarySystem & _sys;

  std::vector<NodalKernelWarehouse> & _nodal_kernels;

  SparseMatrix<Number> & _jacobian;

  /// Number of contributions cached up
  unsigned int _num_cached;
};

#endif //COMPUTENODALKERNELBCJACOBIANSTHREAD_H
