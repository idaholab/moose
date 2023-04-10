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

// Forward declarations
class FEProblemBase;
class NonlinearSystemBase;
class IntegratedBCBase;
class DGKernelBase;
class InterfaceKernelBase;
class TimeKernel;
class KernelBase;
class Kernel;

class ComputeResidualAndJacobianThread : public NonlinearThread
{
public:
  ComputeResidualAndJacobianThread(FEProblemBase & fe_problem,
                                   const std::set<TagID> & vector_tags,
                                   const std::set<TagID> & matrix_tags);

  // Splitting Constructor
  ComputeResidualAndJacobianThread(ComputeResidualAndJacobianThread & x, Threads::split split);

  virtual ~ComputeResidualAndJacobianThread();

  void join(const ComputeResidualAndJacobianThread & /*y*/);

protected:
  using NonlinearThread::compute;
  void compute(ResidualObject & ro) override;
  void accumulateNeighbor() override;
  void accumulateNeighborLower() override;
  void accumulateLower() override;
  void accumulate() override;
  void determineObjectWarehouses() override;

  std::string objectType() const override { return "combined Jacobian & Residual"; }

  /// the tags denoting the vectors we want our residual objects to fill
  const std::set<TagID> & _vector_tags;

  /// the tags denoting the matrices we want our residual objects to fill
  const std::set<TagID> & _matrix_tags;
};
