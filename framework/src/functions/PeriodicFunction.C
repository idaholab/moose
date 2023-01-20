//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeriodicFunction.h"
#include "MooseUtils.h"
#include <iostream>
#include <limits>

registerMooseObject("MooseApp", PeriodicFunction);

InputParameters
PeriodicFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addRequiredParam<FunctionName>(
      "base_function", "The function used as a basis for the generated periodic function.");
  params.addRangeCheckedParam<Real>("period_time",
                                    std::numeric_limits<Real>::max(),
                                    "period_time>0",
                                    "The period for repetition of the base function in time");
  params.addRangeCheckedParam<Real>(
      "period_x",
      std::numeric_limits<Real>::max(),
      "period_x>0",
      "The period for repetition of the base function in the x direction");
  params.addRangeCheckedParam<Real>(
      "period_y",
      std::numeric_limits<Real>::max(),
      "period_y>0",
      "The period for repetition of the base function in the y direction");
  params.addRangeCheckedParam<Real>(
      "period_z",
      std::numeric_limits<Real>::max(),
      "period_z>0",
      "The period for repetition of the base function in the y direction");
  params.addClassDescription(
      "Provides a periodic function by repeating a user-supplied base function in time and/or any "
      "of the three Cartesian coordinate directions");
  return params;
}

PeriodicFunction::PeriodicFunction(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),
    _base_function(getFunctionByName(getParam<FunctionName>("base_function"))),
    _period_time(getParam<Real>("period_time")),
    _period_x(getParam<Real>("period_x")),
    _period_y(getParam<Real>("period_y")),
    _period_z(getParam<Real>("period_z"))
{
}

Real
PeriodicFunction::value(Real t, const Point & p) const
{
  return valueInternal(t, p);
}

ADReal
PeriodicFunction::value(const ADReal & t, const ADPoint & p) const
{
  return valueInternal(t, p);
}

template <typename T, typename P>
T
PeriodicFunction::valueInternal(const T & t, const P & p) const
{
  T t_base = std::fmod(t, _period_time);
  if (t_base < 0.0)
    t_base += _period_time;

  T x_base = std::fmod(p(0), _period_x);
  if (x_base < 0.0)
    x_base += _period_x;

  T y_base = std::fmod(p(1), _period_y);
  if (y_base < 0.0)
    y_base += _period_y;

  T z_base = std::fmod(p(2), _period_z);
  if (z_base < 0.0)
    z_base += _period_z;

  P p_base(x_base, y_base, z_base);

  return _base_function.value(t_base, p_base);
}
