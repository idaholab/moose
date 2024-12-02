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
 * Limits a control value by a range.
 */
class LimitChainControl : public ChainControl
{
public:
  static InputParameters validParams();

  LimitChainControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// Lower bound to apply to control data
  const Real _min_value;
  /// Upper bound to apply to control data
  const Real _max_value;
  /// Control value before limiting
  const Real & _unlimited_value;
  /// Control value after limiting
  Real & _limited_value;
};
