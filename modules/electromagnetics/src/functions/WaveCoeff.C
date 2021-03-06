#include "WaveCoeff.h"
#include "ElkEnums.h"
#include <complex>

registerMooseObject("ElkApp", WaveCoeff);

InputParameters
WaveCoeff::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription(
      "Function for use as coefficient in standard-form Helmholtz wave equation applications.");
  params.addRequiredParam<FunctionName>("eps_rel_real", "Relative permittivity, real component.");
  params.addRequiredParam<FunctionName>("eps_rel_imag",
                                        "Relative permittivity, imaginary component.");
  params.addRequiredParam<FunctionName>("mu_rel_real", "Relative permeability, real component.");
  params.addRequiredParam<FunctionName>("mu_rel_imag",
                                        "Relative permeability, imaginary component.");
  params.addRequiredParam<Real>("k", "Wave number.");
  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>("component", component, "Real or Imaginary wave component.");
  return params;
}

WaveCoeff::WaveCoeff(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _eps_r_real(getFunction("eps_rel_real")),
    _eps_r_imag(getFunction("eps_rel_imag")),
    _mu_r_real(getFunction("mu_rel_real")),
    _mu_r_imag(getFunction("mu_rel_imag")),
    _k(getParam<Real>("k")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
WaveCoeff::value(Real t, const Point & p) const
{
  std::complex<double> eps_r(_eps_r_real.value(t, p), _eps_r_imag.value(t, p));
  std::complex<double> mu_r(_mu_r_real.value(t, p), _mu_r_imag.value(t, p));

  std::complex<double> val = std::pow(_k, 2) * mu_r * eps_r;

  if (_component == elk::REAL)
  {
    return val.real();
  }
  else
  {
    return val.imag();
  }
}
