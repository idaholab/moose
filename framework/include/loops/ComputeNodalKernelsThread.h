//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
class AuxiliarySystem;
class NodalKernelBase;

class ComputeNodalKernelsThread
  : public ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>
{
public:
  ComputeNodalKernelsThread(FEProblemBase & fe_problem,
                            MooseObjectTagWarehouse<NodalKernelBase> & nodal_kernels,
                            const std::set<TagID> & tags);

  // Splitting Constructor
  ComputeNodalKernelsThread(ComputeNodalKernelsThread & x, Threads::split split);

  virtual void pre() override;

  virtual void onNode(ConstNodeRange::const_iterator & node_it) override;

  void join(const ComputeNodalKernelsThread & /*y*/);

protected:
  /// Print execution order of object types in the loop
  void printGeneralExecutionInformation() const override;

  FEProblemBase & _fe_problem;

  AuxiliarySystem & _aux_sys;

  const std::set<TagID> & _tags;

  MooseObjectTagWarehouse<NodalKernelBase> & _nodal_kernels;

  MooseObjectWarehouse<NodalKernelBase> * _nkernel_warehouse;

  /// Number of contributions cached up
  unsigned int _num_cached;
};
