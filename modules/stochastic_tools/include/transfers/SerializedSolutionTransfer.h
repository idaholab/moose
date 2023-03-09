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
#include "ParallelSolutionStorage.h"
#include "StochasticToolsTransfer.h"
#include "UserObjectInterface.h"

// Forward declarations
class ParallelSolutionStorage;

class SerializedSolutionTransfer : public StochasticToolsTransfer
{
public:
  static InputParameters validParams();
  SerializedSolutionTransfer(const InputParameters & parameters);
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
  ParallelSolutionStorage * _parallel_storage;
};
