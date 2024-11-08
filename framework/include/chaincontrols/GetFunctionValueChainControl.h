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

class Function;

/**
 * Creates a control data and populates it by evaluating a Function.
 */
class GetFunctionValueChainControl : public ChainControl
{
public:
  static InputParameters validParams();

  GetFunctionValueChainControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// The new control data
  Real & _value;
  /// Function to be evaluated
  const Function & _function;
  /// Spatial point at which to evaluate the function
  const Point _point;
};
