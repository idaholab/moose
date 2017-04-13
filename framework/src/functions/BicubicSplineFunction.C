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

template <>
InputParameters
validParams<BicubicSplineFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<std::vector<Real>>("x1", "The first independent coordinate.");
  params.addRequiredParam<std::vector<Real>>("x2", "The second independent coordinate.");
  params.addRequiredParam<std::vector<Real>>("y", "The dependent values");
  params.addParam<std::vector<Real>>(
      "yx11", "The values of the derivative wrt x1 on the lower interpolation grid points.");
  params.addParam<std::vector<Real>>(
      "yx1n", "The values of the derivative wrt x1 on the upper interpolation grid points.");
  params.addParam<std::vector<Real>>(
      "yx21", "The values of the derivative wrt x2 on the lower interpolation grid points.");
  params.addParam<std::vector<Real>>(
      "yx2n", "The values of the derivative wrt x2 on the upper interpolation grid points.");
  params.addParam<FunctionName>(
      "yx1", "1e30", "The functional form of the derivative with respect to x1.");
  params.addParam<FunctionName>(
      "yx2", "1e30", "The functional form of the derivative with respect to x2.");

  return params;
}

BicubicSplineFunction::BicubicSplineFunction(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),
    _yx1(getFunction("yx1")),
    _yx2(getFunction("yx2"))
{
  _x1 = getParam<std::vector<Real>>("x1");
  _x2 = getParam<std::vector<Real>>("x2");
  std::vector<Real> yvec = getParam<std::vector<Real>>("y");
  if (isParamValid("yx11"))
    _yx11 = getParam<std::vector<Real>>("yx11");
  if (isParamValid("yx1n"))
    _yx1n = getParam<std::vector<Real>>("yx1n");
  if (isParamValid("yx21"))
    _yx21 = getParam<std::vector<Real>>("yx21");
  if (isParamValid("yx2n"))
    _yx2n = getParam<std::vector<Real>>("yx2n");

  unsigned int m = _x1.size(), n = _x2.size(), mn = yvec.size();
  if (m * n != mn)
    mooseError("The length of the supplied y must be equal to the lengths of x1 and x2 multiplied "
               "together");

  std::vector<std::vector<Real>> y(m, std::vector<Real>(n));
  for (unsigned int i = 0; i < m; ++i)
    for (unsigned int j = 0; j < m; ++j)
      y[i][j] = yvec[i * m + j];

  if (_yx11.empty())
    _yx11.resize(n, 1e30);
  else if (_yx11.size() != n)
    mooseError("The length of the vectors holding the first derivatives of y with respect to x1 "
               "must match the length of x2.");

  if (_yx1n.empty())
    _yx1n.resize(n, 1e30);
  else if (_yx1n.size() != n)
    mooseError("The length of the vectors holding the first derivatives of y with respect to x1 "
               "must match the length of x2.");

  if (_yx21.empty())
    _yx21.resize(m, 1e30);
  else if (_yx21.size() != m)
    mooseError("The length of the vectors holding the first derivatives of y with respect to x2 "
               "must match the length of x1.");

  if (_yx2n.empty())
    _yx2n.resize(m, 1e30);
  else if (_yx2n.size() != m)
    mooseError("The length of the vectors holding the first derivatives of y with respect to x2 "
               "must match the length of x1.");

  _ipol.setData(_x1, _x2, y, _yx11, _yx1n, _yx21, _yx2n);
}

Real
BicubicSplineFunction::value(Real /*t*/, const Point & p)
{
  Point x1(_x1[0], p(1), 0);
  Point xn(_x1.back(), p(1), 0);
  Real yx11 = _yx1.value(0, x1);
  Real yx1n = _yx1.value(0, xn);

  return _ipol.sample(p(0), p(1), yx11, yx1n);
}

Real
BicubicSplineFunction::derivative(const Point & p, unsigned int deriv_var)
{
  Real yp1, ypn;
  if (deriv_var == 1)
  {
    Point x1(_x1[0], p(1), 0);
    Point xn(_x1.back(), p(1), 0);
    yp1 = _yx1.value(0, x1);
    ypn = _yx1.value(0, xn);
  }
  else if (deriv_var == 2)
  {
    Point x1(p(0), _x2[0], 0);
    Point xn(p(0), _x2.back(), 0);
    yp1 = _yx2.value(0, x1);
    ypn = _yx2.value(0, xn);
  }
  else
    mooseError("deriv_var must equal 1 or 2");

  return _ipol.sampleDerivative(p(0), p(1), deriv_var, yp1, ypn);
}

Real
BicubicSplineFunction::secondDerivative(const Point & p, unsigned int deriv_var)
{
  Real yp1, ypn;
  if (deriv_var == 1)
  {
    Point x1(_x1[0], p(1), 0);
    Point xn(_x1.back(), p(1), 0);
    yp1 = _yx1.value(0, x1);
    ypn = _yx1.value(0, xn);
  }
  else if (deriv_var == 2)
  {
    Point x1(p(0), _x2[0], 0);
    Point xn(p(0), _x2.back(), 0);
    yp1 = _yx2.value(0, x1);
    ypn = _yx2.value(0, xn);
  }
  else
    mooseError("deriv_var must equal 1 or 2");

  return _ipol.sample2ndDerivative(p(0), p(1), deriv_var, yp1, ypn);
}
