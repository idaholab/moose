//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorConstantIC.h"
#include "libmesh/point.h"

registerMooseObject("MooseApp", VectorConstantIC);

InputParameters
VectorConstantIC::validParams()
{
  InputParameters params = VectorInitialCondition::validParams();
  params.addRequiredParam<Real>("x_value", "The x value to be set in IC");
  params.addParam<Real>("y_value", 0, "The y value to be set in IC");
  params.addParam<Real>("z_value", 0, "The z value to be set in IC");

  params.addClassDescription("Sets constant component values for a vector field variable.");
  return params;
}

VectorConstantIC::VectorConstantIC(const InputParameters & parameters)
  : VectorInitialCondition(parameters),
    _x_value(getParam<Real>("x_value")),
    _y_value(getParam<Real>("y_value")),
    _z_value(getParam<Real>("z_value"))
{
}

RealVectorValue
VectorConstantIC::value(const Point & /*p*/)
{
  return {_x_value, _y_value, _z_value};
}
