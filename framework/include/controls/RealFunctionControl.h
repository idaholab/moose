//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef REALFUNCTIONCONTROL_H
#define REALFUNCTIONCONTROL_H

// MOOSE includes
#include "Control.h"

// Forward declarations
class RealFunctionControl;
class Function;

template <>
InputParameters validParams<RealFunctionControl>();

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
  RealFunctionControl(const InputParameters & parameters);

  virtual void execute() override;

private:
  /// The function to execute
  Function & _function;

  /// Vector of parameters to change
  ControllableParameter<Real> _parameters;
};

#endif // REALFUNCTIONCONTROL_H
