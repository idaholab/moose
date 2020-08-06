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
#include "PODFullSolveMultiApp.h"

// Forward declarations
class PODFullSolveMultiApp;

/**
 * Transfer solutions from sub-applications to a container in a Trainer.
 * This object also transfers artificial solution vectors back to sub-applications.
 */
class PODSamplerSolutionTransfer : public StochasticToolsTransfer, SurrogateModelInterface
{
public:
  static InputParameters validParams();
  PODSamplerSolutionTransfer(const InputParameters & parameters);
  virtual void initialSetup() override;

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

protected:
  /// The input multiapp casted into a PODFullSolveMultiapp to get access to the
  /// specific pod attributes. Used in batch mode only and checking if the
  /// correct MultiApp type has been provided.
  std::shared_ptr<PODFullSolveMultiApp> _pod_multi_app;

  /**
   * The trainer object to save the solution vector into or to fetch the
   * artificial solution vectors from.
   */
  PODReducedBasisTrainer & _trainer;
};
