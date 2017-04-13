/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "BoundingValueElementDamper.h"

template <>
InputParameters
validParams<BoundingValueElementDamper>()
{
  InputParameters params = validParams<ElementDamper>();
  params.addParam<Real>("max_value",
                        std::numeric_limits<Real>::max(),
                        "The maximum permissible iterative value for the variable.");
  params.addParam<Real>("min_value",
                        std::numeric_limits<Real>::lowest(),
                        "The minimum permissible iterative value for the variable.");
  return params;
}

BoundingValueElementDamper::BoundingValueElementDamper(const InputParameters & parameters)
  : ElementDamper(parameters),
    _max_value(parameters.get<Real>("max_value")),
    _min_value(parameters.get<Real>("min_value"))
{
  if (_min_value > _max_value)
    mooseError("max_value must be greater than min_value");
}

Real
BoundingValueElementDamper::computeQpDamping()
{
  // Note that _u_increment contains the negative of the increment
  if (_u[_qp] < _min_value)
    return 1.0 - (_u[_qp] - _min_value) / -_u_increment[_qp];
  else if (_u[_qp] > _max_value)
    return 1.0 - (_u[_qp] - _max_value) / -_u_increment[_qp];

  return 1.0;
}
