//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatWaveReaction.h"
#include "ElectromagneticEnums.h"
#include "ElectromagneticConstants.h"
#include "Function.h"
#include <complex>

registerMooseObject("ElectromagneticsApp", ADMatWaveReaction);

InputParameters
ADMatWaveReaction::validParams()
{
  InputParameters params = ADVectorKernel::validParams();
  params.addClassDescription(
      "Calculates the current source term in the Helmholtz wave equation using "
      "the dielectric formulation of the current.");
  params.addRequiredCoupledVar("E_real", "The real component of the electric field.");
  params.deprecateParam("E_real", "field_real", "10/01/2025");
  params.addRequiredCoupledVar("field_real", "The real component of the electric field.");
  params.addRequiredCoupledVar("E_imag", "The imaginary component of the electric field.");
  params.deprecateParam("E_imag", "field_imag", "10/01/2025");
  params.addRequiredCoupledVar("field_imag", "The imaginary component of the electric field.");

  params.addParam<MaterialPropertyName>(
      "wave_coef_real",
      "wave_equation_coefficient_real",
      "The real component of the coefficient for the Helmholtz wave equation.");
  params.addParam<MaterialPropertyName>(
      "wave_coef_imag",
      "wave_equation_coefficient_imaginary",
      "The imaginary component of the coefficient for the Helmholtz wave equation.");

  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>("component", component, "Component of field (real or imaginary).");
  return params;
}

ADMatWaveReaction::ADMatWaveReaction(const InputParameters & parameters)
  : ADVectorKernel(parameters),
    _field_real(adCoupledVectorValue("field_real")),
    _field_imag(adCoupledVectorValue("field_imag")),

    _coef_real(getADMaterialProperty<Real>("wave_coef_real")),
    _coef_imag(getADMaterialProperty<Real>("wave_coef_imag")),

    _component(getParam<MooseEnum>("component"))
{
}

ADReal
ADMatWaveReaction::computeQpResidual()
{
  // TODO: In the future, need to add an AD capability to std::complex
  if (_component == EM::REAL)
    return -_test[_i][_qp] *
           (_coef_real[_qp] * _field_real[_qp] - _coef_imag[_qp] * _field_imag[_qp]);
  else
    return -_test[_i][_qp] *
           (_coef_imag[_qp] * _field_real[_qp] + _coef_real[_qp] * _field_imag[_qp]);
}
