#include "HelmholtzTestFunc.h"

template <>
InputParameters
validParams<HelmholtzTestFunc>()
{
  InputParameters params = validParams<Function>();
  params.addClassDescription("Function of analytical solution for use in convergence testing with "
                             "coupled_helmholtz test file.");
  params.addRequiredParam<Real>("L", "Length of 1D test domain, where 0 < x < L");
  params.addRequiredParam<Real>(
      "a", "Real component of squared coefficient c in the PDE, u'' + c^2 * u = 0");
  params.addRequiredParam<Real>(
      "b", "Imaginary component of squared coefficient c in the PDE, u'' + c^2 * u = 0");
  params.addParam<Real>("d", 1.0, "Real component of optional laplacian coefficient");
  params.addParam<Real>("h", 0.0, "Imaginary component of optional laplacian coefficient");
  params.addRequiredParam<Real>("g0_real", "Real component of DirichletBC where x = 0.");
  params.addRequiredParam<Real>("g0_imag", "Imaginary component of DirichletBC where x = 0.");
  params.addRequiredParam<Real>("gL_real", "Real component of DirichletBC where x = L.");
  params.addRequiredParam<Real>("gL_imag", "Imaginary component of DirichletBC where x = L.");
  MooseEnum component("imaginary real", "real");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component.");
  return params;
}

HelmholtzTestFunc::HelmholtzTestFunc(const InputParameters & parameters)
  : Function(parameters),

    _L(getParam<Real>("L")),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),
    _d(getParam<Real>("d")),
    _h(getParam<Real>("h")),
    _g0_real(getParam<Real>("g0_real")),
    _g0_imag(getParam<Real>("g0_imag")),
    _gL_real(getParam<Real>("gL_real")),
    _gL_imag(getParam<Real>("gL_imag")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
HelmholtzTestFunc::value(Real t, const Point & p)
{
  // Initialize angle for calculation of polar form of complex number d - jh
  Real _theta = std::atan2(-_h, _d);

  Real _lambda_real = std::pow((_d * _d + _h * _h), -0.25) *
                      (_a * std::cos(_theta / 2) - _b * std::sin(_theta / 2));

  Real _lambda_imag = std::pow((_d * _d + _h * _h), -0.25) *
                      (_a * std::sin(_theta / 2) + _b * std::cos(_theta / 2));

  Real _coeff = std::pow((std::sin(_lambda_real * _L) * std::cosh(_lambda_imag * _L)), 2) +
                std::pow((std::cos(_lambda_real * _L) * std::sinh(_lambda_imag * _L)), 2);

  _C1_real = _g0_real;
  _C2_real = (_gL_real * std::sin(_lambda_real * _L) * std::cosh(_lambda_imag * _L) +
              _gL_imag * std::cos(_lambda_real * _L) * std::sinh(_lambda_imag * _L) -
              _g0_real * std::cos(_lambda_real * _L) * std::sin(_lambda_real * _L) *
                  std::pow(std::cosh(_lambda_imag * _L), 2) +
              _g0_real * std::cos(_lambda_real * _L) * std::sin(_lambda_real * _L) *
                  std::pow(std::sinh(_lambda_imag * _L), 2) -
              _g0_imag * std::pow(std::cos(_lambda_real * _L), 2) * std::cosh(_lambda_imag * _L) *
                  std::sinh(_lambda_imag * _L) -
              _g0_imag * std::pow(std::sin(_lambda_real * _L), 2) * std::sinh(_lambda_imag * _L) *
                  std::cosh(_lambda_imag * _L)) /
             _coeff;

  _C1_imag = _g0_imag;
  _C2_imag = (-_gL_real * std::cos(_lambda_real * _L) * std::sinh(_lambda_imag * _L) +
              _gL_imag * std::sin(_lambda_real * _L) * std::cosh(_lambda_imag * _L) +
              _g0_real * std::pow(std::cos(_lambda_real * _L), 2) * std::cosh(_lambda_imag * _L) *
                  std::sinh(_lambda_imag * _L) +
              _g0_real * std::pow(std::sin(_lambda_real * _L), 2) * std::sinh(_lambda_imag * _L) *
                  std::cosh(_lambda_imag * _L) -
              _g0_imag * std::cos(_lambda_real * _L) * std::sin(_lambda_real * _L) *
                  std::pow(std::cosh(_lambda_imag * _L), 2) +
              _g0_imag * std::sin(_lambda_real * _L) * std::cos(_lambda_real * _L) *
                  std::pow(std::sinh(_lambda_imag * _L), 2)) /
             _coeff;

  if (_component == "real")
  {
    return _C1_real * std::cos(_lambda_real * p(0)) * std::cosh(_lambda_imag * p(0)) +
           _C1_imag * std::sin(_lambda_real * p(0)) * std::sinh(_lambda_imag * p(0)) +
           _C2_real * std::sin(_lambda_real * p(0)) * std::cosh(_lambda_imag * p(0)) -
           _C2_imag * std::cos(_lambda_real * p(0)) * std::sinh(_lambda_imag * p(0));
  }
  else
  {
    return -_C1_real * std::sin(_lambda_real * p(0)) * std::sinh(_lambda_imag * p(0)) +
           _C1_imag * std::cos(_lambda_real * p(0)) * std::cosh(_lambda_imag * p(0)) +
           _C2_real * std::cos(_lambda_real * p(0)) * std::sinh(_lambda_imag * p(0)) +
           _C2_imag * std::sin(_lambda_real * p(0)) * std::cosh(_lambda_imag * p(0));
  }
}
