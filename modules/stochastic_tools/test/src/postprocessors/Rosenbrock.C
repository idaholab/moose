//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Rosenbrock.h"

registerMooseObject("StochasticToolsTestApp", Rosenbrock);

InputParameters
Rosenbrock::validParams()
{
  return OptimizationTestFunction::validParams();
}

Rosenbrock::Rosenbrock(const InputParameters & parameters) : OptimizationTestFunction(parameters) {}

Real
Rosenbrock::function(const std::vector<Real> & x) const
{
  return rosen(x);
}

Real
Rosenbrock::rosen(const std::vector<Real> & x)
{
  if (x.size() < 2)
    return 0.0;

  PostprocessorValue sum = 0.0;
  for (const auto i : make_range(x.size() - 1))
    sum += 100.0 * Utility::pow<2>(x[i + 1] - x[i] * x[i]) + Utility::pow<2>(1 - x[i]);
  return sum;
}
