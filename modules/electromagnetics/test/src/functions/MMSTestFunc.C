//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MMSTestFunc.h"
#include "ElectromagneticEnums.h"
#include <complex>

registerMooseObject("ElectromagneticsTestApp", MMSTestFunc);

InputParameters
MMSTestFunc::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription(
      "Function of RHS for manufactured solution in scalar_complex_helmholtz test.");
  params.addRequiredParam<Real>("L", "Length of 1D test domain, where 0 < x < L");
  params.addRequiredParam<Real>("g0_real", "Real component of DirichletBC where x = 0.");
  params.addRequiredParam<Real>("g0_imag", "Imaginary component of DirichletBC where x = 0.");
  params.addRequiredParam<Real>("gL_real", "Real component of DirichletBC where x = L.");
  params.addRequiredParam<Real>("gL_imag", "Imaginary component of DirichletBC where x = L.");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component.");
  return params;
}

MMSTestFunc::MMSTestFunc(const InputParameters & parameters)
  : Function(parameters),
    _length(getParam<Real>("L")),
    _g_0_real(getParam<Real>("g0_real")),
    _g_0_imag(getParam<Real>("g0_imag")),
    _g_l_real(getParam<Real>("gL_real")),
    _g_l_imag(getParam<Real>("gL_imag")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
MMSTestFunc::value(Real /*t*/, const Point & p) const
{

  Real val = 0;
  std::complex<double> F(0, 0);

  std::complex<double> g_0(_g_0_real, _g_0_imag);
  std::complex<double> g_l(_g_l_real, _g_l_imag);

  std::complex<double> k(2.0 * (1.0 + p(0) / _length), (1.0 + p(0) / _length));
  std::complex<double> k_l(4.0, 2.0);
  std::complex<double> c(12.0 * (1.0 + p(0) / _length) * (1.0 + p(0) / _length),
                         4.0 * (1.0 + p(0) / _length) * (1.0 + p(0) / _length));
  std::complex<double> c_l(48.0, 16.0);

  std::complex<double> c_grad((24.0 / _length) * (1 + p(0) / _length),
                              (8.0 / _length) * (1 + p(0) / _length));

  std::complex<double> lambda = k / std::sqrt(c);
  std::complex<double> lambda_l = k_l / std::sqrt(c_l);

  std::complex<double> constant_1 = g_0;
  std::complex<double> constant_2 =
      (g_l - g_0 * std::cos(lambda_l * _length)) / std::sin(lambda_l * _length);

  std::complex<double> sln_grad = -constant_1 * lambda * std::sin(lambda * p(0)) +
                                  constant_2 * lambda * std::cos(lambda * p(0));

  F = c_grad * sln_grad;

  if (_component == EM::REAL)
  {
    val = F.real();
  }
  else
  {
    val = F.imag();
  }

  // sign flip because being used in -(cu')' - k^2 * u = -F(x) strong form
  return -val;
}
