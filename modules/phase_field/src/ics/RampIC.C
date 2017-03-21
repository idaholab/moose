/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "RampIC.h"
#include "FEProblem.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<RampIC>()
{
  InputParameters params = validParams<InitialCondition>();
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
