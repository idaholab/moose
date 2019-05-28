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
#include "FunctionInterface.h"

class LinearCombinationFunction;

template <>
InputParameters validParams<LinearCombinationFunction>();

/**
 * Sum_over_i (w_i * functions_i)
 */
class LinearCombinationFunction : public Function, protected FunctionInterface
{
public:
  LinearCombinationFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & pt) const override;
  virtual RealVectorValue vectorValue(Real t, const Point & p) const override;
  virtual RealGradient gradient(Real t, const Point & p) const override;

private:
  std::vector<Real> _w;

  std::vector<Function *> _f;
};
