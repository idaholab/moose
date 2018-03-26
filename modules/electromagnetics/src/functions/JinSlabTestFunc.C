#include "JinSlabTestFunc.h"
#include <complex>

template <>
InputParameters
validParams<JinSlabTestFunc>()
{
  InputParameters params = validParams<Function>();
  params.addClassDescription("Function of analytical solution for use in convergence testing with "
                             "coupled_helmholtz test file.");
  params.addRequiredParam<Real>("length", "Length of 1D test domain, where 0 < x < L");
  params.addRequiredParam<Real>("theta", "Wave incidence angle.");
  params.addParam<FunctionName>("epsR_real", 1.0, "Relative permittivity, real component");
  params.addParam<FunctionName>("epsR_imag", 0.0, "Relative permittivity, imaginary component");
  params.addParam<Real>("muR_real", 1.0, "Relative permeability, real component");
  params.addParam<Real>("muR_imag", 0.0, "Relative permeability, imaginary component");
  params.addRequiredParam<Real>("k", "Wave Number");
  MooseEnum component("imaginary real", "real");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component.");
  return params;
}

JinSlabTestFunc::JinSlabTestFunc(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _L(getParam<Real>("length")),
    _theta(getParam<Real>("theta")),
    _epsR_real(getFunction("epsR_real")),
    _epsR_imag(getFunction("epsR_imag")),
    _muR_real(getParam<Real>("muR_real")),
    _muR_imag(getParam<Real>("muR_imag")),
    _k(getParam<Real>("k")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
JinSlabTestFunc::value(Real t, const Point & p)
{
  std::complex<double> _j(0, 1);
  std::complex<double> _epsR(_epsR_real.value(t, p), _epsR_imag.value(t, p));
  std::complex<double> _muR(_muR_real, _muR_imag);

  std::complex<double> _c =
      std::pow(_k, 2) * (_muR * _epsR - std::sin(_theta * libMesh::pi / 180.) *
                                            std::sin(_theta * libMesh::pi / 180.));

  std::complex<double> _C1 =
      (2 * std::cos(_theta * libMesh::pi / 180.) *
       std::exp(_j * _c * std::cos(_theta * libMesh::pi / 180.) * _L)) /
      (std::exp(_j * _c * _L) * (1 + std::cos(_theta * libMesh::pi / 180.)) +
       std::exp(-_j * _c * _L) * (1 - std::cos(_theta * libMesh::pi / 180.)));

  std::complex<double> _C2 = -_C1;

  std::complex<double> val = _C1 * std::exp(_j * _c * p(0)) + _C2 * std::exp(-_j * _c * p(0));

  if (_component == "real")
  {
    return val.real();
  }
  else
  {
    return val.imag();
  }
}
