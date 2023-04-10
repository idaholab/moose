//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "ThreadedNodeLoop.h"
#include "MooseObjectTagWarehouse.h"

class AuxiliarySystem;
class NodalKernelBase;

class ComputeNodalKernelBcsThread
  : public ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>
{
public:
  ComputeNodalKernelBcsThread(FEProblemBase & fe_problem,
                              MooseObjectTagWarehouse<NodalKernelBase> & nodal_kernels,
                              const std::set<TagID> & tags);
  // Splitting Constructor
  ComputeNodalKernelBcsThread(ComputeNodalKernelBcsThread & x, Threads::split split);

  virtual void pre() override;

  virtual void onNode(ConstBndNodeRange::const_iterator & node_it) override;

  void join(const ComputeNodalKernelBcsThread & /*y*/);

protected:
  /// Print information about the loop, mostly order of execution of objects
  void printGeneralExecutionInformation() const override;

  FEProblemBase & _fe_problem;

  AuxiliarySystem & _aux_sys;

  const std::set<TagID> & _tags;

  MooseObjectTagWarehouse<NodalKernelBase> & _nodal_kernels;

  MooseObjectWarehouse<NodalKernelBase> * _nkernel_warehouse;

  // Number of contributions cached up
  unsigned int _num_cached;
};
