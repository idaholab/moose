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
#include "PODReducedBasisTrainer.h"
#include "SamplerSolutionTransfer.h"

// Forward declarations
class SamplerReceiver;
class PODFullSolveMultiApp;
class StochasticResults;

/**
 * Transfer Postprocessor from sub-applications to a VectorPostprocessor on the master application.
 *
 * This object transfers the distributed data to a StochasticResults object.
 */
class ResidualTransfer : public SamplerSolutionTransfer
{
public:
  static InputParameters validParams();
  ResidualTransfer(const InputParameters & parameters);
  virtual void initialSetup() override;

protected:
  /**
   * Traditional Transfer callback
   */
  virtual void execute() override;

  ///@{
  /**
   * Methods used when running in batch mode (see SamplerFullSolveMultiApp)
   */
  virtual void initializeFromMultiapp() override;
  virtual void executeFromMultiapp() override;
  virtual void finalizeFromMultiapp() override;
  ///@}


};
