//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Tricrystal2CircleGrainsIC.h"
#include "MooseRandom.h"
#include "MooseMesh.h"

registerMooseObject("PhaseFieldApp", Tricrystal2CircleGrainsIC);

InputParameters
Tricrystal2CircleGrainsIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription("Tricrystal with two circles/bubbles");
  params.addRequiredParam<unsigned int>("op_num", "Number of grain order parameters");
  params.addRequiredParam<unsigned int>("op_index", "Index for the current grain order parameter");
  return params;
}

Tricrystal2CircleGrainsIC::Tricrystal2CircleGrainsIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mesh(_fe_problem.mesh()),
    _op_num(getParam<unsigned int>("op_num")),
    _op_index(getParam<unsigned int>("op_index"))
{
  if (_op_num != 3)
    paramError("op_num", "Tricrystal ICs must have op_num = 3");

  // Set up domain bounds with mesh tools
  for (const auto i : make_range(Moose::dim))
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;
}

Real
Tricrystal2CircleGrainsIC::value(const Point & p)
{
  Point grain_center_left;
  grain_center_left(0) = _bottom_left(0) + _range(0) / 4.0;
  grain_center_left(1) = _bottom_left(1) + _range(1) / 2.0;
  grain_center_left(2) = _bottom_left(2) + _range(2) / 2.0;

  Point grain_center_right;
  grain_center_right(0) = _bottom_left(0) + _range(0) * 3.0 / 4.0;
  grain_center_right(1) = _bottom_left(1) + _range(1) / 2.0;
  grain_center_right(2) = _bottom_left(2) + _range(2) / 2.0;

  Real radius = _range(0) / 5.0;
  Real dist_left = (p - grain_center_left).norm();
  Real dist_right = (p - grain_center_right).norm();

  if ((dist_left <= radius && _op_index == 1) || (dist_right <= radius && _op_index == 2) ||
      (dist_left > radius && dist_right > radius && _op_index == 0))
    return 1.0;
  else
    return 0.0;
}
