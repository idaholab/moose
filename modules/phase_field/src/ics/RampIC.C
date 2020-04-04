//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RampIC.h"
#include "FEProblem.h"
#include "MooseMesh.h"

registerMooseObject("PhaseFieldApp", RampIC);

InputParameters
RampIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription(
      "Linear ramp along the x-axis with given values at the left and right extreme points.");
  params.addRequiredParam<Real>("value_left", "The value on left (xmin) boundary.");
  params.addRequiredParam<Real>("value_right", "The value on right (xmax) boundary.");
  return params;
}

RampIC::RampIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _xlength(_fe_problem.mesh().dimensionWidth(0)),
    _xmin(_fe_problem.mesh().getMinInDimension(0)),
    _value_left(getParam<Real>("value_left")),
    _value_right(getParam<Real>("value_right"))
{
}

Real
RampIC::value(const Point & p)
{
  return (_value_right - _value_left) / _xlength * (p(0) - _xmin) + _value_left;
}

RealGradient
RampIC::gradient(const Point & /*p*/)
{
  return (_value_right - _value_left) / _xlength;
}
