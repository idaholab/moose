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
#include "StochasticToolsTransfer.h"

// Forward declarations
class SamplerReceiver;
class SamplerFullSolveMultiApp;
class StochasticResults;

/**
 * Transfer solutions from sub-applications to a container in a Trainer.
 * This object also transfers artificial solution vectors back to sub-applications.
 */
class SamplerSolutionTransfer : public StochasticToolsTransfer
{
public:
  static InputParameters validParams();
  SamplerSolutionTransfer(const InputParameters & parameters);
  virtual void initialSetup() override;

protected:
  /** Name of the trainer object which contains the container for the solutions
   * of the subapp or contains the artificial solution vectors.
   */
  std::string _trainer_name;

  /** The trainer object to save the solution vector into or to fetch the
   * artificial solution vectors from.
   */
  PODReducedBasisTrainer * _trainer = nullptr;

  virtual void execute() override;

  ///@{
  /**
   * Methods used when running in batch mode (see SamplerFullSolveMultiApp)
   */
  virtual void initializeFromMultiapp() override;
  virtual void executeFromMultiapp() override;
  virtual void finalizeFromMultiapp() override;

  virtual void initializeToMultiapp() override;
  virtual void executeToMultiapp() override;
  virtual void finalizeToMultiapp() override;
  ///@}
};
