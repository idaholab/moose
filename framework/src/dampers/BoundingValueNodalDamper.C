//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundingValueNodalDamper.h"

registerMooseObject("MooseApp", BoundingValueNodalDamper);

InputParameters
BoundingValueNodalDamper::validParams()
{
  InputParameters params = NodalDamper::validParams();
  params.addClassDescription("Limits the value of a variable to be within user-specified bounds.");
  params.addParam<Real>("max_value",
                        std::numeric_limits<Real>::max(),
                        "The maximum permissible iterative value for the variable.");
  params.addParam<Real>("min_value",
                        std::numeric_limits<Real>::lowest(),
                        "The minimum permissible iterative value for the variable.");
  return params;
}

BoundingValueNodalDamper::BoundingValueNodalDamper(const InputParameters & parameters)
  : NodalDamper(parameters),
    _max_value(parameters.get<Real>("max_value")),
    _min_value(parameters.get<Real>("min_value"))
{
  if (_min_value > _max_value)
    mooseError("max_value must be greater than min_value");
}

Real
BoundingValueNodalDamper::computeQpDamping()
{
  // Note that _u_increment contains the negative of the increment
  if (_u[_qp] < _min_value)
    return 1.0 - (_u[_qp] - _min_value) / -_u_increment[_qp];
  else if (_u[_qp] > _max_value)
    return 1.0 - (_u[_qp] - _max_value) / -_u_increment[_qp];

  return 1.0;
}
