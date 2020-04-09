//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmoothCircleIC.h"

registerMooseObject("PhaseFieldApp", SmoothCircleIC);

InputParameters
SmoothCircleIC::validParams()
{
  InputParameters params = SmoothCircleBaseIC::validParams();
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
