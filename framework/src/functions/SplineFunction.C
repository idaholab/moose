//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SplineFunction.h"

registerMooseObject("MooseApp", SplineFunction);

defineLegacyParams(SplineFunction);

InputParameters
SplineFunction::validParams()
{
  InputParameters params = PiecewiseTabularBase::validParams();
  params.addClassDescription(
      "Define a spline function from interpolated data defined by input parameters.");
  params.addParam<Real>(
      "yp1", 1e30, "The value of the first derivative of the interpolating function at point 1");
  params.addParam<Real>(
      "ypn", 1e30, "The value of the first derivative of the interpolating function at point n");

  return params;
}

SplineFunction::SplineFunction(const InputParameters & parameters)
  : PiecewiseTabularBase(parameters),
    _ipol(_raw_x, _raw_y, getParam<Real>("yp1"), getParam<Real>("ypn"))
{
}

Real
SplineFunction::value(Real /*t*/, const Point & p) const
{
  return _ipol.sample(_has_axis ? p(_axis) : t);
}

RealGradient
SplineFunction::gradient(Real /*t*/, const Point & p) const
{
  RealGradient grad(0.0);
  grad(0) = derivative(p);
  return grad;
}

Real
SplineFunction::derivative(const Point & p) const
{
  return _ipol.sampleDerivative(_has_axis ? p(_axis) : t);
}

Real
SplineFunction::secondDerivative(const Point & p) const
{
  return _ipol.sample2ndDerivative(_has_axis ? p(_axis) : t);
}
