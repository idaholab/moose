//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "SamplerFullSolveMultiApp.h"
#include "PODReducedBasisTrainer.h"
#include "SamplerInterface.h"
#include "SurrogateModelInterface.h"

#include "StochasticToolsTypes.h"

class PODSamplerSolutionTransfer;
class PODResidualTransfer;

class PODFullSolveMultiApp : public SamplerFullSolveMultiApp, SurrogateModelInterface
{
public:
  static InputParameters validParams();

  PODFullSolveMultiApp(const InputParameters & parameters);

  virtual bool solveStep(Real dt, Real target_time, bool auto_advance = true) override;

  /// Overriding preTransfer to reinit the subappliations if the object needs to be
  /// executed twice.
  virtual void preTransfer(Real dt, Real target_time) override;

  /// Returning the value of the snapshot generation flag.
  bool snapshotGeneration() { return _snapshot_generation; }

protected:
  /// Returning pointers to the solution transfers. Used in batch mode.
  std::vector<std::shared_ptr<PODSamplerSolutionTransfer>>
  getActiveSolutionTransfers(Transfer::DIRECTION direction);

  /// Returning pointers to the solution transfers. Used in batch mode.
  std::vector<std::shared_ptr<PODResidualTransfer>>
  getActiveResidualTransfers(Transfer::DIRECTION direction);

  /// Evaluating the residuals for every tag in the trainer.
  void computeResidual();

  /// Evaluating the residuals for every tag in the trainer in batch mode.
  void computeResidualBatch(Real target_time);

  /// Pointer to the trainer object itself.
  PODReducedBasisTrainer & _trainer;

  /// Switch used to differentiate between snapshot generation and residual
  /// computation. Residual generation is only possible after the snapshot generation
  /// part is complete.
  bool _snapshot_generation;

private:
};
