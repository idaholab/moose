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

#include "StochasticToolsTypes.h"

class StochasticToolsTransfer;

class PODFullSolveMultiApp : public SamplerFullSolveMultiApp
{
public:
  static InputParameters validParams();

  PODFullSolveMultiApp(const InputParameters & parameters);

  virtual bool solveStep(Real dt, Real target_time, bool auto_advance = true) override;

  /// Overriding preTransfer to reinit the subappliations if the object needs to be
  /// executed twice.
  virtual void preTransfer(Real dt, Real target_time) override;

protected:

  /// Name of the trainer object which this MultiApp generates snaphots/residuals
  /// for.
  std::string _trainer_name;

  /// Pointer to the trainer object itself.
  PODReducedBasisTrainer * _trainer = nullptr;

  /// Evaluating the residuals for every tag in the trainer.
  void computeResidual();

  /// Switch used to differentiate between snapshot generation and residual
  /// computation. Residual generation is only possible after the snapshot generation
  /// part is gone.
  bool _snapshot_generation;

private:

};
