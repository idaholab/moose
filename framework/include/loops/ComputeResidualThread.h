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

class ResidualObject;

class ComputeResidualThread : public NonlinearThread
{
public:
  ComputeResidualThread(FEProblemBase & fe_problem, const std::set<TagID> & tags);

  // Splitting Constructor
  ComputeResidualThread(ComputeResidualThread & x, Threads::split split);

  virtual ~ComputeResidualThread();

  void join(const ComputeResidualThread & /*y*/);

protected:
  using NonlinearThread::compute;
  void compute(ResidualObject & ro) override;

  void accumulateNeighbor() override;
  void accumulateNeighborLower() override;
  void accumulateLower() override;
  void accumulate() override;
  void determineObjectWarehouses() override;

  std::string objectType() const override { return "Residual"; }

  /// the tags denoting the vectors we want our residual objects to fill
  const std::set<TagID> & _tags;
};
