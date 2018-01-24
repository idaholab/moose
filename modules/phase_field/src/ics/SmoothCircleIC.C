/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SmoothCircleIC.h"

template <>
InputParameters
validParams<SmoothCircleIC>()
{
  InputParameters params = validParams<SmoothCircleBaseIC>();
  params.addClassDescription("Circle with a smooth interface");
  params.addRequiredParam<Real>("x1", "The x coordinate of the circle center");
  params.addRequiredParam<Real>("y1", "The y coordinate of the circle center");
  params.addParam<Real>("z1", 0.0, "The z coordinate of the circle center");
  params.addRequiredParam<Real>("radius", "The radius of a circle");
  return params;
}

SmoothCircleIC::SmoothCircleIC(const InputParameters & parameters)
  : SmoothCircleBaseIC(parameters),
    _x1(parameters.get<Real>("x1")),
    _y1(parameters.get<Real>("y1")),
    _z1(parameters.get<Real>("z1")),
    _radius(parameters.get<Real>("radius")),
    _center(_x1, _y1, _z1)
{
}

void
SmoothCircleIC::computeCircleRadii()
{
  _radii = {_radius};
}

void
SmoothCircleIC::computeCircleCenters()
{
  _centers = {_center};
}
