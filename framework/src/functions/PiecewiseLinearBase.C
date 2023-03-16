//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseLinearBase.h"

InputParameters
PiecewiseLinearBase::validParams()
{
  InputParameters params = PiecewiseTabularBase::validParams();
  params.addClassDescription("Linearly interpolates between pairs of x-y data");
  return params;
}

PiecewiseLinearBase::PiecewiseLinearBase(const InputParameters & parameters)
  : PiecewiseTabularBase(parameters), _linear_interp(nullptr)
{
}

void
PiecewiseLinearBase::initialSetup()
{
  if (!_linear_interp)
    mooseError("Classes derived from PiecewiseLinearBase need to call buildInterpolation()");
}

void
PiecewiseLinearBase::buildInterpolation(const bool extrap)
{
  // try building a linear interpolation object
  try
  {
    _linear_interp = std::make_unique<LinearInterpolation>(_raw_x, _raw_y, extrap);
  }
  catch (std::domain_error & e)
  {
    mooseError("In PiecewiseLinearBase ", _name, ": ", e.what());
  }
}

Real
PiecewiseLinearBase::value(Real t, const Point & p) const
{
  const auto x = _has_axis ? p(_axis) : t;
  return _scale_factor * _linear_interp->sample(x);
}

ADReal
PiecewiseLinearBase::value(const ADReal & t, const ADPoint & p) const
{
  const auto x = _has_axis ? p(_axis) : t;
  return _scale_factor * _linear_interp->sample(x);
}

Real
PiecewiseLinearBase::timeDerivative(Real t, const Point &) const
{
  return _has_axis ? 0.0 : _scale_factor * _linear_interp->sampleDerivative(t);
}

RealGradient
PiecewiseLinearBase::gradient(Real, const Point & p) const
{
  RealGradient ret;
  if (_has_axis)
    ret(_axis) = _scale_factor * _linear_interp->sampleDerivative(p(_axis));
  return ret;
}

Real
PiecewiseLinearBase::integral() const
{
  return _scale_factor * _linear_interp->integrate();
}

Real
PiecewiseLinearBase::average() const
{
  return integral() /
         (_linear_interp->domain(_linear_interp->getSampleSize() - 1) - _linear_interp->domain(0));
}

void
PiecewiseLinearBase::setData(const std::vector<Real> & x, const std::vector<Real> & y)
{
  PiecewiseTabularBase::setData(x, y);
  buildInterpolation();
}
