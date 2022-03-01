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

InputParameters
SplineFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription(
      "Define a spline function from interpolated data defined by input parameters.");
  MooseEnum component("x=0 y=1 z=2", "x");
  params.addParam<MooseEnum>(
      "component", component, "The component of the geometry point to interpolate with");
  params.addRequiredParam<std::vector<Real>>("x", "The abscissa values");
  params.addRequiredParam<std::vector<Real>>("y", "The ordinate values");
  params.addParam<Real>(
      "yp1", 1e30, "The value of the first derivative of the interpolating function at point 1");
  params.addParam<Real>(
      "ypn", 1e30, "The value of the first derivative of the interpolating function at point n");

  return params;
}

SplineFunction::SplineFunction(const InputParameters & parameters)
  : Function(parameters),
    _ipol(getParam<std::vector<Real>>("x"),
          getParam<std::vector<Real>>("y"),
          getParam<Real>("yp1"),
          getParam<Real>("ypn")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
SplineFunction::value(Real /*t*/, const Point & p) const
{
  return _ipol.sample(p(_component));
}

ADReal
SplineFunction::value(const ADReal & /*t*/, const ADPoint & p) const
{
  return _ipol.sample(p(_component));
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
  return _ipol.sampleDerivative(p(_component));
}

Real
SplineFunction::secondDerivative(const Point & p) const
{
  return _ipol.sample2ndDerivative(p(_component));
}
