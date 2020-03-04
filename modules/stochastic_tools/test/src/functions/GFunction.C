//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GFunction.h"

registerMooseObject("StochasticToolsTestApp", GFunction);

InputParameters
GFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addParam<std::vector<Real>>(
      "q_vector", std::vector<Real>({0, 0.5, 3, 9, 99, 99}), "q values for GFunction");
  params.addParam<std::vector<Real>>("x_vector", "x values for GFunction");
  params.declareControllable("x_vector");
  return params;
}

GFunction::GFunction(const InputParameters & parameters)
  : Function(parameters),
    _q_vector(getParam<std::vector<Real>>("q_vector")),
    _x_vector(getParam<std::vector<Real>>("x_vector"))
{
  if (_q_vector.size() != _x_vector.size())
    paramError("q_vector", "The 'q_vector' and 'x_vector' must be the same size.");
}

Real
GFunction::value(Real /*t*/, const Point & /*p*/) const
{
  Real y = 1;
  for (std::size_t i = 0; i < _x_vector.size(); ++i)
    y *= (std::abs(4 * _x_vector[i] - 2) + _q_vector[i]) / (1 + _q_vector[i]);
  return y;
}
