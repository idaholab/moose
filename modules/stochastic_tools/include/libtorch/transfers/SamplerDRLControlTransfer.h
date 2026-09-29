//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#pragma once

// torch-based includes
#include "LibtorchDRLControlTrainer.h"

#include "StochasticToolsTransfer.h"
#include "SurrogateModelInterface.h"

class LibtorchDRLControl;

/**
 * Transfers a trained DRL controller from a trainer to controls on sampler subapps.
 */
class SamplerDRLControlTransfer : public StochasticToolsTransfer, public SurrogateModelInterface
{
public:
  /**
   * Return the input parameters supported by this transfer.
   */
  static InputParameters validParams();

  /**
   * Build the transfer that pushes a trained controller into subapps.
   * @param parameters Input parameters for the transfer.
   */
  SamplerDRLControlTransfer(const InputParameters & parameters);

  /**
   * Execute the transfer in the standard non-batch path.
   */
  virtual void execute() override;

  /**
   * Prepare the transfer for batch-mode execution.
   */
  virtual void initialSetup() override;

  /**
   * Initialize a batch transfer from the multiapp.
   */
  virtual void initializeFromMultiapp() override;

  /**
   * Execute a batch transfer from the multiapp.
   */
  virtual void executeFromMultiapp() override;

  /**
   * Finalize a batch transfer from the multiapp.
   */
  virtual void finalizeFromMultiapp() override;

  /**
   * Initialize a batch transfer to the multiapp.
   */
  virtual void initializeToMultiapp() override;

  /**
   * Execute a batch transfer to the multiapp.
   */
  virtual void executeToMultiapp() override;

  /**
   * Finalize a batch transfer to the multiapp.
   */
  virtual void finalizeToMultiapp() override;

protected:
  /**
   * Find the DRL control on a local subapp and check that its control period, observation
   * history length, number of observations, and number of control signals match the trainer's.
   * @param app_index Global index of the local subapp.
   * @return The DRL control on the subapp.
   */
  LibtorchDRLControl & getDRLControl(unsigned int app_index);

  /// The name of the control object on the other app where we want to copy our neural net
  const std::string _control_name;

  /// The trainer object which will contains the control neural net
  const LibtorchDRLControlTrainer & _trainer;
};

#endif
