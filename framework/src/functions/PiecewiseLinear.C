//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseLinear.h"

template <>
InputParameters
validParams<PiecewiseLinear>()
{
  InputParameters params = validParams<Piecewise>();
  params.addClassDescription("Linearly interpolates between pairs of x-y data");
  return params;
}

PiecewiseLinear::PiecewiseLinear(const InputParameters & parameters) : Piecewise(parameters) {}

Real
PiecewiseLinear::value(Real t, const Point & p)
{
  Real func_value;
  if (_has_axis)
  {
    func_value = _linear_interp->sample(p(_axis));
  }
  else
  {
    func_value = _linear_interp->sample(t);
  }
  return _scale_factor * func_value;
}

Real
PiecewiseLinear::timeDerivative(Real t, const Point & p)
{
  Real func_value;
  if (_has_axis)
  {
    func_value = _linear_interp->sampleDerivative(p(_axis));
  }
  else
  {
    func_value = _linear_interp->sampleDerivative(t);
  }
  return _scale_factor * func_value;
}

Real
PiecewiseLinear::integral()
{
  return _scale_factor * _linear_interp->integrate();
}

Real
PiecewiseLinear::average()
{
  return integral() /
         (_linear_interp->domain(_linear_interp->getSampleSize() - 1) - _linear_interp->domain(0));
}
