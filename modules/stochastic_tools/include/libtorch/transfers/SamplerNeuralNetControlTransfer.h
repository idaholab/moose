//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once

// torch-based includes
#include "LibtorchDRLControlTrainer.h"

#include "StochasticToolsTransfer.h"
#include "SurrogateModelInterface.h"

class SamplerNeuralNetControlTransfer : public StochasticToolsTransfer, public SurrogateModelInterface
{
public:
  static InputParameters validParams();

  SamplerNeuralNetControlTransfer(const InputParameters & parameters);

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
  /// The name of the control object on the other app where we want to copy our neural net
  const std::string _control_name;

  /// The trainer object which will contains the control neural net
  const LibtorchDRLControlTrainer & _trainer;
};

#endif
