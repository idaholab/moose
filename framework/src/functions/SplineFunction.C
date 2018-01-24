/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SplineFunction.h"

template <>
InputParameters
validParams<SplineFunction>()
{
  InputParameters params = validParams<Function>();
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
SplineFunction::value(Real /*t*/, const Point & p)
{
  return _ipol.sample(p(_component));
}

RealGradient
SplineFunction::gradient(Real /*t*/, const Point & p)
{
  RealGradient grad(0.0);
  grad(0) = derivative(p);
  return grad;
}

Real
SplineFunction::derivative(const Point & p)
{
  return _ipol.sampleDerivative(p(_component));
}

Real
SplineFunction::secondDerivative(const Point & p)
{
  return _ipol.sample2ndDerivative(p(_component));
}
