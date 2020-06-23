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

  virtual void preTransfer(Real dt, Real target_time) override;

protected:

  std::string _trainer_name;

  /// The trainer object to save the solution into
  PODReducedBasisTrainer * _trainer = nullptr;

  void computeResidual();

  void computeResidualBatch();

private:

};
