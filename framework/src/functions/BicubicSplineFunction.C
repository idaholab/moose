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

#include "BicubicSplineFunction.h"

template<>
InputParameters validParams<BicubicSplineFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<std::vector<Real> >("x1", "The first independent coordinate.");
  params.addRequiredParam<std::vector<Real> >("x2", "The second independent coordinate.");
  params.addRequiredParam<std::vector<Real> >("y", "The dependent values");

  return params;
}

BicubicSplineFunction::BicubicSplineFunction(const InputParameters & parameters) :
    Function(parameters)
{
  std::vector<Real> x1 = getParam<std::vector<Real> >("x1");
  std::vector<Real> x2 = getParam<std::vector<Real> >("x2");
  std::vector<Real> yvec = getParam<std::vector<Real> >("yvec");

  unsigned int m = x1.size(), n = x2.size(), mn = yvec.size();
  if (m * n != mn)
    mooseError("The length of the supplied y must be equal to the lengths of x1 and x2 multiplied together");

  std::vector<std::vector<Real> > y(m, std::vector<Real>(n));
  for (unsigned int i = 0; i < m; ++i)
    for (unsigned int j = 0; j < m; ++j)
      y[i][j] = yvec[i * m + j];

  _ipol.setData(x1, x2, y);
}

Real
BicubicSplineFunction::value(Real /*t*/, const Point & p)
{
  return _ipol.sample(p(0), p(1));
}

Real
BicubicSplineFunction::derivative(const Point & p, unsigned int deriv_var)
{
  return _ipol.sampleDerivative(p(0), p(1), deriv_var);
}

Real
BicubicSplineFunction::secondDerivative(const Point & p, unsigned int deriv_var)
{
  return _ipol.sample2ndDerivative(p(0), p(1), deriv_var);
}
