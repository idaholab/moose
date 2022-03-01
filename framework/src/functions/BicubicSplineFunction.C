//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BicubicSplineFunction.h"

registerMooseObject("MooseApp", BicubicSplineFunction);

InputParameters
BicubicSplineFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription(
      "Define a bicubic spline function from interpolated data defined by input parameters.");

  MooseEnum normal_component("x=0 y=1 z=2", "z");
  params.addParam<MooseEnum>(
      "normal_component",
      normal_component,
      "The component of the geometry that is normal to the spline x1/x2 values");
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
    _normal_component(getParam<MooseEnum>("normal_component")),
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
  unsigned int k = 0;
  for (unsigned int i = 0; i < m; ++i)
    for (unsigned int j = 0; j < n; ++j)
      y[i][j] = yvec[k++];

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

  if (_normal_component == 0)
  {
    // YZ plane
    _x1_index = 1;
    _x2_index = 2;
  }
  else if (_normal_component == 1)
  {
    // ZX plane
    _x1_index = 2;
    _x2_index = 0;
  }
  else
  {
    // XY plane
    _x1_index = 0;
    _x2_index = 1;
  }
}

Real
BicubicSplineFunction::value(Real /*t*/, const Point & p) const
{
  // Call yx11/yx1n with the correctly oriented points
  Real x1_begin = _x1[0];
  Real x1_end = p(_x2_index);
  Real xn_begin = _x1.back();
  Real xn_end = p(_x2_index);

  Point x1(0, 0, 0);
  Point xn(0, 0, 0);

  x1(_x1_index) = x1_begin;
  x1(_x2_index) = x1_end;
  xn(_x1_index) = xn_begin;
  xn(_x2_index) = xn_end;

  Real yx11 = _yx1.value(0, x1);
  Real yx1n = _yx1.value(0, xn);

  return _ipol.sample(p(_x1_index), p(_x2_index), yx11, yx1n);
}

Real
BicubicSplineFunction::derivative(const Point & p, unsigned int deriv_var) const
{
  Real yp1, ypn;
  Point x1(0, 0, 0);
  Point xn(0, 0, 0);
  if (deriv_var == 1)
  {
    // Call yx11/yx1n with the correctly oriented points
    Real x1_begin = _x1[0];
    Real x1_end = p(_x2_index);
    Real xn_begin = _x1.back();
    Real xn_end = p(_x2_index);

    x1(_x1_index) = x1_begin;
    x1(_x2_index) = x1_end;
    xn(_x1_index) = xn_begin;
    xn(_x2_index) = xn_end;

    yp1 = _yx1.value(0, x1);
    ypn = _yx1.value(0, xn);
  }
  else if (deriv_var == 2)
  {
    // Call yx11/yx1n with the correctly oriented points
    Real x1_begin = p(_x1_index);
    Real x1_end = _x2[0];
    Real xn_begin = p(_x1_index);
    Real xn_end = _x2.back();

    x1(_x1_index) = x1_begin;
    x1(_x2_index) = x1_end;
    xn(_x1_index) = xn_begin;
    xn(_x2_index) = xn_end;

    yp1 = _yx2.value(0, x1);
    ypn = _yx2.value(0, xn);
  }
  else
    mooseError("deriv_var must equal 1 or 2");

  return _ipol.sampleDerivative(p(_x1_index), p(_x2_index), deriv_var, yp1, ypn);
}

RealGradient
BicubicSplineFunction::gradient(Real /*t*/, const Point & p) const
{
  RealGradient grad = RealGradient(0, 0, 0);

  Real dF_dx1 = derivative(p, 1);
  Real dF_dx2 = derivative(p, 2);

  grad(_x1_index) = dF_dx1;
  grad(_x2_index) = dF_dx2;

  return grad;
}

Real
BicubicSplineFunction::secondDerivative(const Point & p, unsigned int deriv_var) const
{
  Real yp1, ypn;
  Point x1(0, 0, 0);
  Point xn(0, 0, 0);
  if (deriv_var == 1)
  {
    // Call yx11/yx1n with the correctly oriented points
    Real x1_begin = _x1[0];
    Real x1_end = p(_x2_index);
    Real xn_begin = _x1.back();
    Real xn_end = p(_x2_index);

    x1(_x1_index) = x1_begin;
    x1(_x2_index) = x1_end;
    xn(_x1_index) = xn_begin;
    xn(_x2_index) = xn_end;

    yp1 = _yx1.value(0, x1);
    ypn = _yx1.value(0, xn);
  }
  else if (deriv_var == 2)
  {
    // Call yx11/yx1n with the correctly oriented points
    Real x1_begin = p(_x1_index);
    Real x1_end = _x2[0];
    Real xn_begin = p(_x1_index);
    Real xn_end = _x2.back();

    x1(_x1_index) = x1_begin;
    x1(_x2_index) = x1_end;
    xn(_x1_index) = xn_begin;
    xn(_x2_index) = xn_end;

    yp1 = _yx2.value(0, x1);
    ypn = _yx2.value(0, xn);
  }
  else
    mooseError("deriv_var must equal 1 or 2");

  return _ipol.sample2ndDerivative(p(_x1_index), p(_x2_index), deriv_var, yp1, ypn);
}
