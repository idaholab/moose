//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

/**
 * Defines an initial condition that forces the value to be a user specified function
 */
class FunctionIC : public InitialCondition
{
public:
  static InputParameters validParams();

  FunctionIC(const InputParameters & parameters);

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

  /**
   * The value of the gradient at a point.
   */
  virtual RealGradient gradient(const Point & p) override;

  /// Function to evaluate to form the initial condition
  const Function & _func;

  /// Scaling factor, to be able to use a function with multiple ICs
  const Real _scaling;
};
