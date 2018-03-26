#include "JinSlabCoeffFunc.h"
#include <complex>

template <>
InputParameters
validParams<JinSlabCoeffFunc>()
{
  InputParameters params = validParams<Function>();
  params.addParam<FunctionName>("epsR_real", 1.0, "Relative permittivity, real component");
  params.addParam<FunctionName>("epsR_imag", 0.0, "Relative permittivity, imaginary component");
  params.addParam<Real>("muR_real", 1.0, "Relative permeability, real component");
  params.addParam<Real>("muR_imag", 0.0, "Relative permeability, imaginary component");
  params.addRequiredParam<Real>("k", "Wave Number");
  params.addRequiredParam<Real>("theta", "Wave Incidence angle, in degrees");
  MooseEnum component("imaginary real", "real");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component");
  return params;
}

JinSlabCoeffFunc::JinSlabCoeffFunc(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _epsR_real(getFunction("epsR_real")),
    _epsR_imag(getFunction("epsR_imag")),
    _muR_real(getParam<Real>("muR_real")),
    _muR_imag(getParam<Real>("muR_imag")),
    _k(getParam<Real>("k")),
    _theta(getParam<Real>("theta")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
JinSlabCoeffFunc::value(Real t, const Point & p)
{
  std::complex<double> _epsR(_epsR_real.value(t, p), _epsR_imag.value(t, p));
  std::complex<double> _muR(_muR_real, _muR_imag);

  std::complex<double> _val =
      _k * std::sqrt(_epsR * _muR -
                     std::sin(_theta * libMesh::pi / 180.) * std::sin(_theta * libMesh::pi / 180.));

  if (_component == "real")
  {
    return _val.real();
  }
  else
  {
    return _val.imag();
  }
}
