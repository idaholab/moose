//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"

/**
 * Base class for functions used in inverse optimization
 * The parameterDerivative function is used in adjoint calculation to compute
 * gradients.
 */
class OptimizationFunction : public Function
{
public:
  static InputParameters validParams();

  OptimizationFunction(const InputParameters & parameters);

  virtual std::vector<Real> parameterGradient(Real t, const Point & pt) const = 0;
};
