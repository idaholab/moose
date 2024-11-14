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
 * Creates a constant bool control data.
 */
class ConstantBoolChainControl : public ChainControl
{
public:
  static InputParameters validParams();

  ConstantBoolChainControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// The constant value
  const bool _constant_value;
  /// The new control data
  bool & _value;
};
