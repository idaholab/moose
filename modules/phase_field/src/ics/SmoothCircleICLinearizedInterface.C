//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmoothCircleICLinearizedInterface.h"

registerMooseObject("PhaseFieldApp", SmoothCircleICLinearizedInterface);

InputParameters
SmoothCircleICLinearizedInterface::validParams()
{
  InputParameters params = SmoothCircleIC::validParams();
  params.addClassDescription(
      "Circle with a smooth interface transformed using the linearized interface function");
  params.addRequiredParam<Real>(
      "bound_value", "Bound value used to keep variable between +/-bound. Must be positive.");
  return params;
}

SmoothCircleICLinearizedInterface::SmoothCircleICLinearizedInterface(
    const InputParameters & parameters)
  : SmoothCircleIC(parameters), _bound(parameters.get<Real>("bound_value"))
{
}

Real
SmoothCircleICLinearizedInterface::value(const Point & p)
{
  // Transform bound into the order parameter space
  const Real trans_bound = (1.0 + std::tanh(-_bound / std::sqrt(2.0))) / 2.0;

  // Get IC value for the order parameter
  const Real trans_value = SmoothCircleIC::value(p);

  // Transform order parameter value into linearized interface variable
  Real value = 0.0;

  if (trans_value < trans_bound)
    value = -_bound;
  else if (trans_value > 1.0 - trans_bound)
    value = _bound;
  else
    value = std::sqrt(2.0) * std::atanh(2.0 * trans_value - 1.0);

  return value;
}
