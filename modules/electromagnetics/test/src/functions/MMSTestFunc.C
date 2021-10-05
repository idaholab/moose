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
      "Function of RHS for manufactured solution in spatial_constant_helmholtz test.");
  params.addRequiredParam<Real>("L", "Length of 1D test domain, where 0 < x < L");
  params.addRequiredParam<FunctionName>("a",
                                        "Real component constant of function of squared "
                                        "coefficient c in the PDE, -u'' - c^2 * u = -F");
  params.addRequiredParam<FunctionName>(
      "b", "Imaginary component function of squared coefficient c in the PDE, -u'' - c^2 * u = -F");
  params.addParam<Real>("d", 1.0, "Real component of optional laplacian coefficient");
  params.addParam<Real>("h", 0.0, "Imaginary component of optional laplacian coefficient");
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
    FunctionInterface(this),

    _length(getParam<Real>("L")),
    _a(getFunction("a")),
    _b(getFunction("b")),
    _d(getParam<Real>("d")),
    _h(getParam<Real>("h")),
    _g_0_real(getParam<Real>("g0_real")),
    _g_0_imag(getParam<Real>("g0_imag")),
    _g_l_real(getParam<Real>("gL_real")),
    _g_l_imag(getParam<Real>("gL_imag")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
MMSTestFunc::value(Real t, const Point & p) const
{

  Real val = 0;
  std::complex<double> F(0, 0);

  std::complex<double> g_0(_g_0_real, _g_0_imag);
  std::complex<double> g_l(_g_l_real, _g_l_imag);

  Real a_grad = _a.gradient(t, p)(0);
  Real b_grad = _b.gradient(t, p)(0);
  // assuming a constant * (1 + x / _length)^2 structure for functions
  Real a_second = 2 * _a.value(t, 0) / (_length * _length);
  Real b_second = 2 * _b.value(t, 0) / (_length * _length);

  std::complex<double> c(_a.value(t, p), _b.value(t, p));
  std::complex<double> c_l(_a.value(t, _length), _b.value(t, _length));

  std::complex<double> c_grad(a_grad, b_grad);
  std::complex<double> c_second(a_second, b_second);

  std::complex<double> c_1 = g_0;

  std::complex<double> c_2 = (g_l - g_0 * std::cos(c_l * _length)) / std::sin(c_l * _length);

  F = -c_1 * ((c_second * p(0) + 2.0 * c_grad) * std::sin(c * p(0)) +
              (std::pow(c_grad * p(0), 2) + 2.0 * c_grad * c * p(0)) * std::cos(c * p(0))) +
      c_2 * ((c_second * p(0) + 2.0 * c_grad) * std::cos(c * p(0)) -
             (std::pow(c_grad * p(0), 2) + 2.0 * c_grad * c * p(0)) * std::sin(c * p(0)));

  if (_component == electromagnetics::REAL)
  {
    val = F.real();
  }
  else
  {
    val = F.imag();
  }

  // sign flip because being used in -u'' - c^2 * u = -F(x) strong form
  return -val;
}
