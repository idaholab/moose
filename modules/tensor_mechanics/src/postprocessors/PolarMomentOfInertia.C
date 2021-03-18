//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolarMomentOfInertia.h"

registerMooseObject("TensorMechanicsApp", PolarMomentOfInertia);

InputParameters
PolarMomentOfInertia::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredParam<Point>("origin", "Axis origin");
  params.addRequiredParam<RealVectorValue>("direction", "Axis direction");
  params.addClassDescription(
      "Compute the polar moment of inertia of a sideset w.r.t. a point and a direction");
  return params;
}

PolarMomentOfInertia::PolarMomentOfInertia(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _origin(getParam<Point>("origin")),
    _direction(getParam<RealVectorValue>("direction"))
{
  // normalize direction
  _direction /= _direction.norm();
}

Real
PolarMomentOfInertia::computeQpIntegral()
{
  auto dr = _q_point[_qp] - _origin;
  const auto projection = _direction * (_direction * dr);
  dr -= projection;

  return dr.norm_sq();
}
