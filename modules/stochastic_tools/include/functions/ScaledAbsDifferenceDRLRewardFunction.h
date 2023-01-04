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

/// A simple reward function which uses c1*|x-x_target|+c2
class ScaledAbsDifferenceDRLRewardFunction : public Function, protected FunctionInterface
{
public:
  static InputParameters validParams();

  ScaledAbsDifferenceDRLRewardFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) const override;
  virtual ADReal value(const ADReal & t, const ADPoint & p) const override;

protected:
  /// Value we would like to reach (can be time and space dependent)
  const Function & _design_function;

  /// Postprocessor containing the observed value
  const PostprocessorValue & _observed_value;

  /// Coefficients for the reward function
  const Real & _c1;
  const Real & _c2;
};
