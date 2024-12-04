//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ChainControl.h"

/**
 * Terminates the simulation when a boolean chain control data has a certain value.
 */
class TerminateChainControl : public ChainControl
{
public:
  static InputParameters validParams();

  TerminateChainControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /**
   * Performs termination
   */
  void terminate();

  /// Whether to terminate on true or false
  const bool _terminate_on_true;

  /// Flag to throw an error if the terminate condition is met
  const bool _throw_error;

  /// Message to use if termination occurs
  const std::string & _termination_message;

  /// The control data that indicates if the simulation should be terminated
  const bool & _input;
};
