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

template<>
InputParameters validParams<SplineFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<std::vector<Real> >("x", "The abscissa values");
  params.addRequiredParam<std::vector<Real> >("y", "The ordinate values");
  params.addParam<Real>("yp1", 1e30, "The value of the first derivative of the interpolating function at point 1");
  params.addParam<Real>("ypn", 1e30, "The value of the first derivative of the interpolating function at point n");

  return params;
}

SplineFunction::SplineFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
    _ipol(getParam<std::vector<Real> >("x"),
          getParam<std::vector<Real> >("y"),
          getParam<Real>("yp1"),
          getParam<Real>("ypn"))
{
}

SplineFunction::~SplineFunction()
{
}

Real
SplineFunction::value(Real /*t*/, const Point & p)
{
  return _ipol.sample(p(0));
}

Real
SplineFunction::derivative(const Point & p)
{
  return _ipol.sampleDerivative(p(0));
}

Real
SplineFunction::secondDerivative(const Point & p)
{
  return _ipol.sample2ndDerivative(p(0));
}
