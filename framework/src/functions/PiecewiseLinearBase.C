//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseLinearBase.h"

template <typename BaseClass>
InputParameters
PiecewiseLinearBaseTempl<BaseClass>::validParams()
{
  InputParameters params = BaseClass::validParams();
  params.addClassDescription("Linearly interpolates between pairs of x-y data");
  return params;
}

template <typename BaseClass>
PiecewiseLinearBaseTempl<BaseClass>::PiecewiseLinearBaseTempl(const InputParameters & parameters)
  : BaseClass(parameters), _linear_interp(nullptr)
{
}

template <typename BaseClass>
void
PiecewiseLinearBaseTempl<BaseClass>::initialSetup()
{
  if (!_linear_interp)
    mooseError("Classes derived from PiecewiseLinearBase need to call buildInterpolation()");
}

template <typename BaseClass>
void
PiecewiseLinearBaseTempl<BaseClass>::buildInterpolation(const bool extrap)
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

template <typename BaseClass>
Real
PiecewiseLinearBaseTempl<BaseClass>::value(Real t, const Point & p) const
{
  const auto x = _has_axis ? p(_axis) : t;
  return _scale_factor * _linear_interp->sample(x);
}

template <typename BaseClass>
ADReal
PiecewiseLinearBaseTempl<BaseClass>::value(const ADReal & t, const ADPoint & p) const
{
  const auto x = _has_axis ? p(_axis) : t;
  return _scale_factor * _linear_interp->sample(x);
}

template <typename BaseClass>
Real
PiecewiseLinearBaseTempl<BaseClass>::timeDerivative(Real t, const Point &) const
{
  return _has_axis ? 0.0 : _scale_factor * _linear_interp->sampleDerivative(t);
}

template <typename BaseClass>
RealGradient
PiecewiseLinearBaseTempl<BaseClass>::gradient(Real, const Point & p) const
{
  RealGradient ret;
  if (_has_axis)
    ret(_axis) = _scale_factor * _linear_interp->sampleDerivative(p(_axis));
  return ret;
}

template <typename BaseClass>
Real
PiecewiseLinearBaseTempl<BaseClass>::integral() const
{
  return _scale_factor * _linear_interp->integrate();
}

template <typename BaseClass>
Real
PiecewiseLinearBaseTempl<BaseClass>::average() const
{
  return integral() /
         (_linear_interp->domain(_linear_interp->getSampleSize() - 1) - _linear_interp->domain(0));
}

template <typename BaseClass>
void
PiecewiseLinearBaseTempl<BaseClass>::setData(const std::vector<Real> & x,
                                             const std::vector<Real> & y)
{
  BaseClass::setData(x, y);
  buildInterpolation();
}

template class PiecewiseLinearBaseTempl<PiecewiseTabularBase>;
template class PiecewiseLinearBaseTempl<ADPiecewiseTabularBase>;
