//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVInitialConditionTempl.h"

#include <string>

class Function;
class InputParameters;

template <typename T>
InputParameters validParams();

/**
 * Defines a boundary condition that forces the value to be a user specified
 * function at the boundary.
 */
class FVFunctionIC : public FVInitialCondition
{
public:
  static InputParameters validParams();

  FVFunctionIC(const InputParameters & parameters);

  /**
   * @returns The function name
   */
  const FunctionName functionName() const;

protected:
  /**
   * Evaluate the function at the current quadrature point and time step.
   */
  Real f();

  /**
   * The value of the variable at a point.
   */
  virtual Real value(const Point & p) override;

  /// Function to evaluate to form the initial condition
  const Function & _func;

  /// Scaling factor, to be able to use a function with multiple ICs
  const Real _scaling;
};
