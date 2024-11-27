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
 * Converts a Real-valued chain control data to boolean.
 */
class RealToBoolChainControl : public ChainControl
{
public:
  static InputParameters validParams();

  RealToBoolChainControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// The value before conversion
  const Real & _input;
  /// The converted value
  bool & _output;
};
