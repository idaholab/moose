//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Eggholder.h"

registerMooseObject("StochasticToolsTestApp", Eggholder);

InputParameters
Eggholder::validParams()
{
  return OptimizationTestFunction::validParams();
}

Eggholder::Eggholder(const InputParameters & parameters) : OptimizationTestFunction(parameters) {}

Real
Eggholder::function(const std::vector<Real> & x) const
{
  if (x.size() != 2)
    paramError("x", "Eggholder problem requires exactly 2 inputs, but ", x.size(), " were given.");

  return eggholder(x);
}

Real
Eggholder::eggholder(const std::vector<Real> & x)
{

  mooseAssert(x.size() == 2, "Eggholder problem requires exactly 2 inputs.");

  return -(x[1] + 47.0) * std::sin(std::sqrt(std::abs(x[0] / 2.0 + (x[1] + 47.0)))) -
         x[0] * std::sin(std::sqrt(abs(x[0] - (x[1] + 47.0))));
}
