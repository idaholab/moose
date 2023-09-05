//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalColoringICLinearizedInterface.h"

registerMooseObject("PhaseFieldApp", PolycrystalColoringICLinearizedInterface);

InputParameters
PolycrystalColoringICLinearizedInterface::validParams()
{
  InputParameters params = PolycrystalColoringIC::validParams();
  params.addClassDescription("Sets up polycrystal initial conditions from user objects for "
                             "transformed linearized interface");
  params.addRequiredParam<Real>(
      "bound_value", "Bound value used to keep variable between +/-bound. Must be positive.");

  return params;
}

PolycrystalColoringICLinearizedInterface::PolycrystalColoringICLinearizedInterface(
    const InputParameters & parameters)
  : PolycrystalColoringIC(parameters), _bound(parameters.get<Real>("bound_value"))
{
}

Real
PolycrystalColoringICLinearizedInterface::value(const Point & p)
{
  // Transform bound into the order parameter space
  const Real trans_bound = (1.0 + std::tanh(-_bound / std::sqrt(2.0))) / 2.0;

  // Get IC value for the order parameter
  const Real trans_value = PolycrystalColoringIC::value(p);

  Real value = 0.0;
  if (trans_value < trans_bound)
    value = -_bound;
  else if (trans_value > 1.0 - trans_bound)
    value = _bound;
  else
    value = std::sqrt(2.0) * std::atanh(2.0 * trans_value - 1.0);

  return value;
}
