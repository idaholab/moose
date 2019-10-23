//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseLinear.h"

registerMooseObject("MooseApp", PiecewiseLinear);

defineLegacyParams(PiecewiseLinear);

InputParameters
PiecewiseLinear::validParams()
{
  InputParameters params = PiecewiseBase::validParams();
  params.addClassDescription("Linearly interpolates between pairs of x-y data");
  return params;
}

PiecewiseLinear::PiecewiseLinear(const InputParameters & parameters) : PiecewiseBase(parameters)
{
  // try building a linear interpolation object
  try
  {
    _linear_interp = libmesh_make_unique<LinearInterpolation>(_raw_x, _raw_y);
  }
  catch (std::domain_error & e)
  {
    mooseError("In PiecewiseLinear ", _name, ": ", e.what());
  }
}

Real
PiecewiseLinear::value(Real t, const Point & p) const
{
  const Real x = _has_axis ? p(_axis) : t;
  return _scale_factor * _linear_interp->sample(x);
}

Real
PiecewiseLinear::timeDerivative(Real t, const Point & p) const
{
  const Real x = _has_axis ? p(_axis) : t;
  return _scale_factor * _linear_interp->sampleDerivative(x);
}

Real
PiecewiseLinear::integral() const
{
  return _scale_factor * _linear_interp->integrate();
}

Real
PiecewiseLinear::average() const
{
  return integral() /
         (_linear_interp->domain(_linear_interp->getSampleSize() - 1) - _linear_interp->domain(0));
}
