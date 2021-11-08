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
  MooseEnum axis("x=0 y=1 z=2 t=3");
  InputParameters params = PiecewiseTabularBase::validParams();
  params.addClassDescription(
      "Define a spline function from interpolated data defined by input parameters.");
  params.addParam<Real>(
      "yp1", 1e30, "The value of the first derivative of the interpolating function at point 1");
  params.addParam<Real>(
      "ypn", 1e30, "The value of the first derivative of the interpolating function at point n");
  params.setDocString("axis",
                      "The axis used (x, y, or z) if this is to be a function of position. "
                      "If this is to be a time-dependent function, t may also be specified "
                      "or omitted. The default value is x");
  axis = "x";
  params.setParameters("axis", axis);
  //  params.setParameters("axis",axis);
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
  return _ipol.sample(_has_axis ? p(_axis) : _t);
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
  return _ipol.sampleDerivative(_has_axis ? p(_axis) : _t);
}

Real
SplineFunction::secondDerivative(const Point & p) const
{
  return _ipol.sample2ndDerivative(_has_axis ? p(_axis) : _t);
}
