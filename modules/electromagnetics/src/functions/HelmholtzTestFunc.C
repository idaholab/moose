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

  Real _coeff = std::pow((std::sin(_a * _L) * std::cosh(_b * _L)), 2) +
                std::pow((std::cos(_a * _L) * std::sinh(_b * _L)), 2);

  _C1_real = _g0_real;
  _C2_real = (_gL_real * std::sin(_a * _L) * std::cosh(_b * _L) +
              _gL_imag * std::cos(_a * _L) * std::sinh(_b * _L) -
              _g0_real * std::cos(_a * _L) * std::sin(_a * _L) *
                  std::pow(std::cosh(_b * _L), 2) +
              _g0_real * std::cos(_a * _L) * std::sin(_a * _L) *
                  std::pow(std::sinh(_b * _L), 2) -
              _g0_imag * std::pow(std::cos(_a * _L), 2) * std::cosh(_b * _L) *
                  std::sinh(_b * _L) -
              _g0_imag * std::pow(std::sin(_a * _L), 2) * std::sinh(_b * _L) *
                  std::cosh(_b * _L)) /
             _coeff;

  _C1_imag = _g0_imag;
  _C2_imag = (-_gL_real * std::cos(_a * _L) * std::sinh(_b * _L) +
              _gL_imag * std::sin(_a * _L) * std::cosh(_b * _L) +
              _g0_real * std::pow(std::cos(_a * _L), 2) * std::cosh(_b * _L) *
                  std::sinh(_b * _L) +
              _g0_real * std::pow(std::sin(_a * _L), 2) * std::sinh(_b * _L) *
                  std::cosh(_b * _L) -
              _g0_imag * std::cos(_a * _L) * std::sin(_a * _L) *
                  std::pow(std::cosh(_b * _L), 2) +
              _g0_imag * std::sin(_a * _L) * std::cos(_a * _L) *
                  std::pow(std::sinh(_b * _L), 2)) /
             _coeff;

  if (_component == "real")
  {
    return _C1_real * std::cos(_a * p(0)) * std::cosh(_b * p(0)) +
           _C1_imag * std::sin(_a * p(0)) * std::sinh(_b * p(0)) +
           _C2_real * std::sin(_a * p(0)) * std::cosh(_b * p(0)) -
           _C2_imag * std::cos(_a * p(0)) * std::sinh(_b * p(0));
  }
  else
  {
    return -_C1_real * std::sin(_a * p(0)) * std::sinh(_b * p(0)) +
           _C1_imag * std::cos(_a * p(0)) * std::cosh(_b * p(0)) +
           _C2_real * std::cos(_a * p(0)) * std::sinh(_b * p(0)) +
           _C2_imag * std::sin(_a * p(0)) * std::cosh(_b * p(0));
  }
}
