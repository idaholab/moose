//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationTestFunction.h"

registerMooseObject("StochasticToolsTestApp", OptimizationTestFunction);

InputParameters
OptimizationTestFunction::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  MooseEnum funcs("rosen eggholder");
  params.addRequiredParam<MooseEnum>("function", funcs, "Type of function to evaluate.");
  params.addRequiredParam<std::vector<Real>>("x", "Input values.");
  params.declareControllable("x");
  return params;
}

OptimizationTestFunction::OptimizationTestFunction(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _function(getParam<MooseEnum>("function")),
    _x(getParam<std::vector<Real>>("x"))
{
}

PostprocessorValue
OptimizationTestFunction::getValue() const
{
  if (_function == "rosen")
    return rosen(_x);
  else if (_function == "eggholder")
    return eggholder(_x);
  else
    paramError("function", "Unknown function: ", _function);
}

Real
OptimizationTestFunction::rosen(const std::vector<Real> & x)
{
  if (x.size() < 2)
    return 0.0;

  PostprocessorValue sum = 0.0;
  for (const auto i : make_range(x.size() - 1))
    sum += 100.0 * Utility::pow<2>(x[i + 1] - x[i] * x[i]) + Utility::pow<2>(1 - x[i]);
  return sum;
}

Real
OptimizationTestFunction::eggholder(const std::vector<Real> & x)
{
  if (x.size() != 2)
    ::mooseError("Eggholder problem requires exactly 2 inputs, but ", x.size(), " were given.");

  return -(x[1] + 47.0) * std::sin(std::sqrt(std::abs(x[0] / 2.0 + (x[1] + 47.0)))) -
         x[0] * std::sin(std::sqrt(abs(x[0] - (x[1] + 47.0))));
}
