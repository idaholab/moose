//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WaveEquationCoefficient.h"
#include "ElectromagneticEnums.h"
#include <complex>

registerMooseObject("ElectromagneticsApp", WaveEquationCoefficient);

InputParameters
WaveEquationCoefficient::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addClassDescription("Material for use as coefficient $a k^2 \\mu_r \\epsilon_r$ (where a "
                             "is a scalar coefficient) "
                             "in standard-form Helmholtz wave equation applications with "
                             "derivatives calculated using automatic differentiation.");
  params.addRequiredParam<MaterialPropertyName>("eps_rel_real",
                                                "Relative permittivity, real component.");
  params.addRequiredParam<MaterialPropertyName>("eps_rel_imag",
                                                "Relative permittivity, imaginary component.");
  params.addRequiredParam<MaterialPropertyName>("mu_rel_real",
                                                "Relative permeability, real component.");
  params.addRequiredParam<MaterialPropertyName>("mu_rel_imag",
                                                "Relative permeability, imaginary component.");
  params.addRequiredParam<MaterialPropertyName>("k_real", "Wave number, real component.");
  params.addParam<MaterialPropertyName>("k_imag", 0, "Wave number, imaginary component.");
  params.addParam<Real>("coef", 1.0, "Real-valued function coefficient.");
  params.addParam<MaterialPropertyName>(
      "prop_name_real",
      "wave_equation_coefficient_real",
      "User-specified material property name for the real component.");
  params.addParam<MaterialPropertyName>(
      "prop_name_imaginary",
      "wave_equation_coefficient_imaginary",
      "User-specified material property name for the imaginary component.");
  return params;
}

WaveEquationCoefficient::WaveEquationCoefficient(const InputParameters & parameters)
  : ADMaterial(parameters),
    _eps_r_real(getADMaterialProperty<Real>("eps_rel_real")),
    _eps_r_imag(getADMaterialProperty<Real>("eps_rel_imag")),
    _mu_r_real(getADMaterialProperty<Real>("mu_rel_real")),
    _mu_r_imag(getADMaterialProperty<Real>("mu_rel_imag")),
    _k_real(getADMaterialProperty<Real>("k_real")),
    _k_imag(getADMaterialProperty<Real>("k_imag")),
    _coef(getParam<Real>("coef")),
    _prop_name_real(getParam<MaterialPropertyName>("prop_name_real")),
    _prop_name_imag(getParam<MaterialPropertyName>("prop_name_imaginary")),
    _prop_real(declareADProperty<Real>(_prop_name_real)),
    _prop_imag(declareADProperty<Real>(_prop_name_imag))
{
}

void
WaveEquationCoefficient::computeQpProperties()
{
  ADReal k_sq_real = (_k_real[_qp] * _k_real[_qp]) - (_k_imag[_qp] * _k_imag[_qp]);
  ADReal k_sq_imag = 2.0 * _k_real[_qp] * _k_imag[_qp];

  ADReal mu_eps_real = (_mu_r_real[_qp] * _eps_r_real[_qp]) - (_mu_r_imag[_qp] * _eps_r_imag[_qp]);
  ADReal mu_eps_imag = (_mu_r_real[_qp] * _eps_r_imag[_qp]) + (_mu_r_imag[_qp] * _eps_r_real[_qp]);

  _prop_real[_qp] = _coef * ((k_sq_real * mu_eps_real) - (k_sq_imag * mu_eps_imag));
  _prop_imag[_qp] = _coef * ((k_sq_real * mu_eps_imag) + (k_sq_imag * mu_eps_real));
}
