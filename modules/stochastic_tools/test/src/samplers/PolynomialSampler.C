//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolynomialSampler.h"

registerMooseObject("StochasticToolsTestApp", PolynomialSampler);

InputParameters
PolynomialSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addRequiredParam<std::vector<Real>>(
      "coefficients",
      "The polynomial coefficients ($a_i$) for $y = a_0 + a_1x a_2x^2 + \\ldots + a_nx^n$.");
  params.addParam<Real>("x_min", 0, "The minimum value of x to use.");
  params.addParam<Real>("x_max", 1, "The maximum value of x to use.");
  params.addParam<dof_id_type>("num_rows", 100, "The number of x,y pairs to create.");
  params.addParam<Real>(
      "noise",
      0.1,
      "The standard deviation of normally distributed random noise to apply to the y values.");
  return params;
}

PolynomialSampler::PolynomialSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _x_min(getParam<Real>("x_min")),
    _x_max(getParam<Real>("x_max")),
    _noise(getParam<Real>("noise")),
    _coefficients(getParam<std::vector<Real>>("coefficients"))
{
  setNumberOfRandomSeeds(2);
  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(2);
}

void
PolynomialSampler::advanceGenerators(dof_id_type count)
{
  for (dof_id_type i = 0; i < count; ++i)
  {
    getRand(0);
    getRandNormal(1, 0, _noise);
  }
}

void
PolynomialSampler::computeSampleRow(dof_id_type /*i*/, std::vector<Real> & data)

{
  Real x = getRand(0) * (_x_max - _x_min) + _x_min;
  Real y = 0;
  for (std::size_t i = 0; i < _coefficients.size(); ++i)
    y += _coefficients[i] * std::pow(x, i);
  data[0] = x;
  data[1] = y + getRandNormal(1, 0, _noise);
}
