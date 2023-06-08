//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "THMControl.h"

/**
 * This control block will terminate a run if its input indicates so.
 */
class TerminateControl : public THMControl
{
public:
  TerminateControl(const InputParameters & parameters);

  virtual void execute();

protected:
  /// Flag to throw an error if the terminate condition is met
  const bool _throw_error;

  /// Message to use if termination occurs
  const std::string & _termination_message;

  /// The control data that indicates if the simulation should be terminated
  const bool & _terminate;

public:
  static InputParameters validParams();
};
