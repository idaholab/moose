//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundingBoxIC.h"
#include "libmesh/point.h"

registerMooseObject("MooseApp", BoundingBoxIC);

InputParameters
BoundingBoxIC::validParams()
{
  InputParameters params = InitialConditionTempl<Real>::validParams();
  params.addRequiredParam<Real>("x1", "The x coordinate of the lower left-hand corner of the box");
  params.addRequiredParam<Real>("y1", "The y coordinate of the lower left-hand corner of the box");
  params.addParam<Real>("z1", 0.0, "The z coordinate of the lower left-hand corner of the box");

  params.addRequiredParam<Real>("x2", "The x coordinate of the upper right-hand corner of the box");
  params.addRequiredParam<Real>("y2", "The y coordinate of the upper right-hand corner of the box");
  params.addParam<Real>("z2", 0.0, "The z coordinate of the upper right-hand corner of the box");

  params.addParam<Real>("inside", 0.0, "The value of the variable inside the box");
  params.addParam<Real>("outside", 0.0, "The value of the variable outside the box");

  params.addParam<Real>(
      "int_width", 0.0, "The width of the diffuse interface. Set to 0 for sharp interface.");

  params.addClassDescription("BoundingBoxIC allows setting the initial condition of a value inside "
                             "and outside of a specified box. The box is aligned with the x, y, z "
                             "axes");

  return params;
}

BoundingBoxIC::BoundingBoxIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _x1(getParam<Real>("x1")),
    _y1(getParam<Real>("y1")),
    _z1(getParam<Real>("z1")),
    _x2(getParam<Real>("x2")),
    _y2(getParam<Real>("y2")),
    _z2(getParam<Real>("z2")),
    _inside(getParam<Real>("inside")),
    _outside(getParam<Real>("outside")),
    _bottom_left(_x1, _y1, _z1),
    _top_right(_x2, _y2, _z2),
    _int_width(getParam<Real>("int_width"))
{
}

Real
BoundingBoxIC::value(const Point & p)
{
  if (_int_width < 0.0)
    mooseError("'int_width' should be non-negative");

  if (_int_width == 0.0)
  {
    for (const auto i : make_range(Moose::dim))
      if (p(i) < _bottom_left(i) || p(i) > _top_right(i))
        return _outside;

    return _inside;
  }
  else
  {
    Real f_in = 1.0;
    for (const auto i : make_range(Moose::dim))
      if (_bottom_left(i) != _top_right(i))
        f_in *= 0.5 * (std::tanh(2.0 * (p(i) - _bottom_left(i)) / _int_width) -
                       std::tanh(2.0 * (p(i) - _top_right(i)) / _int_width));

    return _outside + (_inside - _outside) * f_in;
  }
}
