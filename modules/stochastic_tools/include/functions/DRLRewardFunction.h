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

class DRLRewardFunction : public Function, protected FunctionInterface
{
public:
  static InputParameters validParams();

  DRLRewardFunction(const InputParameters & parameters);

  // using Function::value;
  virtual Real value(Real t, const Point & p) const override;
  virtual ADReal value(const ADReal & t, const ADPoint & p) const override;

protected:
  /// value we would like to reach (can be time and spatial dependent)
  const Function & _design_function;

  /// postprocessor containing the observed value
  const PostprocessorName & _observed_pp_name;
  const PostprocessorValue & _observed_value;

  /*
    /// postprocessor containing the action value
    const PostprocessorName & _action_pp_name;
    const PostprocessorValue & _action_value;
  */

  /// coefficients in the reward function, c1 - c2
  /// c1 * exp(-c2 * abs(diff))
  const Real & _c1;
  const Real & _c2;
};
