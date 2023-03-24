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
  void compute(ResidualObject & ro) override;
  void accumulateNeighbor() override;
  void accumulateNeighborLower() override;
  void accumulateLower() override;
  void accumulate() override;
  void determineResidualObjects() override;

  /// Print information about the loop, mostly order of execution of objects
  void printGeneralExecutionInformation() const override;

  /// Print list of specific objects executed and in which order
  void printBlockExecutionInformation() const override;

  /// the tags denoting the vectors we want our residual objects to fill
  const std::set<TagID> & _tags;
};
