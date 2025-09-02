//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedNodeLoop.h"
#include "MooseObjectTagWarehouse.h"

#include "libmesh/node_range.h"

// Forward declarations
class FEProblemBase;
class NonlinearSystemBase;
class AuxiliarySystem;
class NodalKernelBase;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class SparseMatrix;
}

class ComputeNodalKernelJacobiansThread final
  : public ThreadedNodeLoop<ConstNodeRange,
                            ConstNodeRange::const_iterator,
                            ComputeNodalKernelJacobiansThread>
{
public:
  ComputeNodalKernelJacobiansThread(FEProblemBase & fe_problem,
                                    NonlinearSystemBase & nl,
                                    MooseObjectTagWarehouse<NodalKernelBase> & nodal_kernels,
                                    const std::set<TagID> & tags);

  // Splitting Constructor
  ComputeNodalKernelJacobiansThread(ComputeNodalKernelJacobiansThread & x, Threads::split split);

  void pre();

  void onNode(ConstNodeRange::const_iterator & node_it);

  void join(const ComputeNodalKernelJacobiansThread & /*y*/);

  /// Print information about the loop, mostly order of execution of objects
  void printGeneralExecutionInformation() const;

protected:
  FEProblemBase & _fe_problem;
  NonlinearSystemBase & _nl;
  AuxiliarySystem & _aux_sys;

  const std::set<TagID> & _tags;

  MooseObjectTagWarehouse<NodalKernelBase> & _nodal_kernels;

  MooseObjectWarehouse<NodalKernelBase> * _nkernel_warehouse;

  /// Number of contributions cached up
  unsigned int _num_cached;
};
