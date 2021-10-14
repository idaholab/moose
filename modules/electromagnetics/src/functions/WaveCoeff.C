//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WaveCoeff.h"
#include "ElectromagneticEnums.h"
#include <complex>

registerMooseObject("ElectromagneticsApp", WaveCoeff);

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
  params.addRequiredParam<FunctionName>("k_real", "Wave number, real component.");
  params.addParam<FunctionName>("k_imag", 0, "Wave number, imaginary component.");
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
    _k_real(getFunction("k_real")),
    _k_imag(getFunction("k_imag")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
WaveCoeff::value(Real t, const Point & p) const
{
  std::complex<double> eps_r(_eps_r_real.value(t, p), _eps_r_imag.value(t, p));
  std::complex<double> mu_r(_mu_r_real.value(t, p), _mu_r_imag.value(t, p));
  std::complex<double> k(_k_real.value(t, p), _k_imag.value(t, p));

  std::complex<double> val = k * k * mu_r * eps_r;

  if (_component == electromagnetics::REAL)
  {
    return val.real();
  }
  else
  {
    return val.imag();
  }
}
