//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CubicTransitionFunction.h"

registerMooseObject("ThermalHydraulicsApp", CubicTransitionFunction);

InputParameters
CubicTransitionFunction::validParams()
{
  InputParameters params = SmoothTransitionFunction::validParams();

  params.addRequiredParam<Real>("function1_derivative_end_point", "First function");
  params.addRequiredParam<Real>("function2_derivative_end_point", "Second function");

  params.addClassDescription("Computes a cubic polynomial transition between two functions");

  return params;
}

CubicTransitionFunction::CubicTransitionFunction(const InputParameters & parameters)
  : SmoothTransitionFunction(parameters),

    _df1dx_end_point(getParam<Real>("function1_derivative_end_point")),
    _df2dx_end_point(getParam<Real>("function2_derivative_end_point")),

    _transition(_x_center, _transition_width)
{
  Point p1, p2;
  Real t1 = 0.0, t2 = 0.0;
  if (_use_time)
  {
    t1 = _transition.leftEnd();
    t2 = _transition.rightEnd();
  }
  else
  {
    p1(_component) = _transition.leftEnd();
    p2(_component) = _transition.rightEnd();
  }

  _transition.initialize(
      _function1.value(t1, p1), _function2.value(t2, p2), _df1dx_end_point, _df2dx_end_point);
}

Real
CubicTransitionFunction::value(Real t, const Point & p) const
{
  const Real x = _use_time ? t : p(_component);

  return _transition.value(x, _function1.value(t, p), _function2.value(t, p));
}

RealVectorValue
CubicTransitionFunction::gradient(Real /*t*/, const Point & /*p*/) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " is not implemented.");
}
