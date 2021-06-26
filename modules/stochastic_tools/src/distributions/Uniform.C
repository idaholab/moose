//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Uniform.h"

registerMooseObject("StochasticToolsApp", Uniform);

InputParameters
Uniform::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("Continuous uniform distribution.");
  params.addParam<Real>("lower_bound", 0.0, "Distribution lower bound");
  params.addParam<Real>("upper_bound", 1.0, "Distribution upper bound");
  return params;
}

Uniform::Uniform(const InputParameters & parameters)
  : Distribution(parameters),
    _lower_bound(getParam<Real>("lower_bound")),
    _upper_bound(getParam<Real>("upper_bound"))
{
  if (_lower_bound >= _upper_bound)
    mooseError("The lower bound is larger than the upper bound!");
}

Real
Uniform::pdf(const Real & x, const Real & lower_bound, const Real & upper_bound)
{
  if (x < lower_bound || x > upper_bound)
    return 0.0;
  else
    return 1.0 / (upper_bound - lower_bound);
}

Real
Uniform::cdf(const Real & x, const Real & lower_bound, const Real & upper_bound)
{
  if (x < lower_bound)
    return 0.0;
  else if (x > upper_bound)
    return 1.0;
  else
    return (x - lower_bound) / (upper_bound - lower_bound);
}

Real
Uniform::quantile(const Real & y, const Real & lower_bound, const Real & upper_bound)
{
  if (y < 0 || y > 1)
    ::mooseError("The cdf_value provided is out of range 0 to 1.");
  else
    return y * (upper_bound - lower_bound) + lower_bound;
}

Real
Uniform::pdf(const Real & x) const
{
  return pdf(x, _lower_bound, _upper_bound);
}

Real
Uniform::cdf(const Real & x) const
{
  return cdf(x, _lower_bound, _upper_bound);
}

Real
Uniform::quantile(const Real & y) const
{
  return quantile(y, _lower_bound, _upper_bound);
}
