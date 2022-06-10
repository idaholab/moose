//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RndBoundingBoxIC.h"
#include "MooseRandom.h"

registerMooseObject("PhaseFieldApp", RndBoundingBoxIC);

InputParameters
RndBoundingBoxIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription(
      "Random noise with different min/max inside/outside of a bounding box");

  params.addRequiredParam<Real>("x1", "The x coordinate of the lower left-hand corner of the box");
  params.addRequiredParam<Real>("y1", "The y coordinate of the lower left-hand corner of the box");
  params.addParam<Real>("z1", 0.0, "The z coordinate of the lower left-hand corner of the box");

  params.addRequiredParam<Real>("x2", "The x coordinate of the upper right-hand corner of the box");
  params.addRequiredParam<Real>("y2", "The y coordinate of the upper right-hand corner of the box");
  params.addParam<Real>("z2", 0.0, "The z coordinate of the upper right-hand corner of the box");

  params.addRequiredParam<Real>("mx_invalue", "The max value of the variable invalue the box");
  params.addRequiredParam<Real>("mx_outvalue", "The max value of the variable outvalue the box");

  params.addParam<Real>("mn_invalue", 0.0, "The min value of the variable invalue the box");
  params.addParam<Real>("mn_outvalue", 0.0, "The min value of the variable outvalue the box");
  return params;
}

RndBoundingBoxIC::RndBoundingBoxIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _x1(parameters.get<Real>("x1")),
    _y1(parameters.get<Real>("y1")),
    _z1(parameters.get<Real>("z1")),
    _x2(parameters.get<Real>("x2")),
    _y2(parameters.get<Real>("y2")),
    _z2(parameters.get<Real>("z2")),
    _mx_invalue(parameters.get<Real>("mx_invalue")),
    _mx_outvalue(parameters.get<Real>("mx_outvalue")),
    _mn_invalue(parameters.get<Real>("mn_invalue")),
    _mn_outvalue(parameters.get<Real>("mn_outvalue")),
    _range_invalue(_mx_invalue - _mn_invalue),
    _range_outvalue(_mx_outvalue - _mn_outvalue),
    _bottom_left(_x1, _y1, _z1),
    _top_right(_x2, _y2, _z2)
{
  mooseAssert(_range_invalue >= 0.0, "Inside Min > Inside Max for RandomIC!");
  mooseAssert(_range_outvalue >= 0.0, "Outside Min > Outside Max for RandomIC!");
}

Real
RndBoundingBoxIC::value(const Point & p)
{
  // Random number between 0 and 1
  Real rand_num = MooseRandom::rand();

  for (const auto i : make_range(Moose::dim))
    if (p(i) < _bottom_left(i) || p(i) > _top_right(i))
      return rand_num * _range_outvalue + _mn_outvalue;

  return rand_num * _range_invalue + _mn_invalue;
}
