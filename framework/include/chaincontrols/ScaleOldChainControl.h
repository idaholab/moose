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
 * Scales an old control value by another control value.
 */
class ScaleOldChainControl : public ChainControl
{
public:
  static InputParameters validParams();

  ScaleOldChainControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// Control value after scaling
  Real & _value;
  /// Control value before scaling
  const Real & _value_old;
  /// Factor by which to scale control value
  const Real & _scale_factor;
};
