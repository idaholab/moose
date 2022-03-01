//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Control.h"

class Function;

/**
 * A basic control for changing an input parameter using a Function
 */
class RealFunctionControl : public Control
{
public:
  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  static InputParameters validParams();

  RealFunctionControl(const InputParameters & parameters);

  virtual void execute() override;

private:
  /// The function to execute
  const Function & _function;
};
