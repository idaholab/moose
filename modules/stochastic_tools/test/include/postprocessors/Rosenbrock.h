//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

#include "OptimizationTestFunction.h"

class Rosenbrock : public OptimizationTestFunction
{
public:
  static InputParameters validParams();
  Rosenbrock(const InputParameters & parameters);

  /// https://en.wikipedia.org/wiki/Test_functions_for_optimization
  static Real rosen(const std::vector<Real> & x);

protected:
  Real function(const std::vector<Real> & x) const override final;
};
