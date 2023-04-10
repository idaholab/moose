//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NonlinearThread.h"
#include "MooseObjectTagWarehouse.h"

#include "libmesh/elem_range.h"

// Forward declarations
class FEProblemBase;
class NonlinearSystemBase;
class IntegratedBCBase;
class DGKernelBase;
class InterfaceKernelBase;
class Kernel;
class FVElementalKernel;

class ComputeJacobianThread : public NonlinearThread
{
public:
  ComputeJacobianThread(FEProblemBase & fe_problem, const std::set<TagID> & tags);

  // Splitting Constructor
  ComputeJacobianThread(ComputeJacobianThread & x, Threads::split split);

  virtual ~ComputeJacobianThread();

  virtual void postElement(const Elem * /*elem*/) override;

  void join(const ComputeJacobianThread & /*y*/);

protected:
  const std::set<TagID> & _tags;

  void determineObjectWarehouses() override;
  void accumulateNeighbor() override;
  virtual void accumulateNeighborLower() override;
  virtual void accumulateLower() override;
  virtual void accumulate() override{};

  virtual void compute(ResidualObject &) override { mooseError("Not implemented"); };
  void compute(KernelBase & kernel) override;
  void compute(FVElementalKernel & kernel) override;
  void compute(IntegratedBCBase & bc) override;
  void compute(DGKernelBase & dg, const Elem * neighbor) override;
  void compute(InterfaceKernelBase & ik) override;

  std::string objectType() const override { return "Jacobian"; }
};
