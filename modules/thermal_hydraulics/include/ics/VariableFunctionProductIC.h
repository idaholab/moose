//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

class Function;

/**
 * Computes product of a variable and a function
 */
class VariableFunctionProductIC : public InitialCondition
{
public:
  VariableFunctionProductIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// Coupled variable
  const VariableValue & _var;
  /// Function
  const Function & _fn;

public:
  static InputParameters validParams();
};
