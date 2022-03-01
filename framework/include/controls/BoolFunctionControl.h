//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Control.h"

class Function;

/**
 * A basic control for changing a boolean-valued input parameter using a Function
 */
class BoolFunctionControl : public Control
{
public:
  static InputParameters validParams();

  BoolFunctionControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// The function to determine the value of the controlled parameter
  const Function & _function;
};
