//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef LIBTORCH_ENABLED
#include "LibtorchDRLControl.h"
#endif

#include "GeneralPostprocessor.h"
#include "InputParameterWarehouse.h"

/**
 * A class for querying output signals from LibtorchNeuralNetControl and
 * derived objects.
 */
class LibtorchDRLLogProbabilityPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  LibtorchDRLLogProbabilityPostprocessor(const InputParameters & parameters);

  ///@{
  /**
   * No action taken
   */
  void initialize() override {}
  void execute() override {}
  ///@}

  /// We override this to setup the linking withthe control object. We need to do it here
  /// because the PPs are contructed before te Control objects.
  void initialSetup() override;

  /**
   * Returns the value of the latest response of a neural-network-based controller.
   * This means that we grab current response value stored wihtin the controller.
   */
  virtual Real getValue() override;

private:
  unsigned int _signal_index;

#ifdef LIBTORCH_ENABLED
  // This can't be const beause PPs are constructed before Controls
  std::shared_ptr<LibtorchDRLControl> _libtorch_nn_control;
#endif
};
