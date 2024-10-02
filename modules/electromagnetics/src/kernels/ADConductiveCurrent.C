//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConductiveCurrent.h"
#include "ElectromagneticEnums.h"
#include "ElectromagneticConstants.h"
#include <complex>

registerMooseObject("ElectromagneticsApp", ADConductiveCurrent);

InputParameters
ADConductiveCurrent::validParams()
{
  InputParameters params = ADVectorKernel::validParams();
  params.addClassDescription(
      "Calculates the current source term in the Helmholtz wave equation using "
      "the conduction formulation of the current.");
  params.addRequiredCoupledVar("E_real", "The real component of the E-field.");
  params.addRequiredCoupledVar("E_imag", "The imaginary component of the E-field.");

  params.addParam<MaterialPropertyName>(
      "conductivity_real", 1.0, "The real component of the material conductivity.");
  params.addParam<MaterialPropertyName>(
      "conductivity_imag", 0.0, "The imaginary component of the material conductivity.");

  params.addParam<MaterialPropertyName>("ang_freq_real", "ang_freq", "The drive frequency.");
  params.addParam<MaterialPropertyName>("ang_freq_imag", 0.0, "The drive frequency.");

  params.addParam<MaterialPropertyName>("permeability_real", "mu_vacuum", "The permeability.");
  params.addParam<MaterialPropertyName>("permeability_imag", 0.0, "The permeability.");

  MooseEnum component("real imaginary");
  params.addParam<MooseEnum>("component", component, "Component of field (real or imaginary).");
  return params;
}

ADConductiveCurrent::ADConductiveCurrent(const InputParameters & parameters)
  : ADVectorKernel(parameters),
    _E_real(adCoupledVectorValue("E_real")),
    _E_imag(adCoupledVectorValue("E_imag")),

    _cond_real(getADMaterialProperty<Real>("conductivity_real")),
    _cond_imag(getADMaterialProperty<Real>("conductivity_imag")),

    _omega_real(getADMaterialProperty<Real>("ang_freq_real")),
    _omega_imag(getADMaterialProperty<Real>("ang_freq_imag")),

    _mu_real(getADMaterialProperty<Real>("permeability_real")),
    _mu_imag(getADMaterialProperty<Real>("permeability_imag")),

    _component(getParam<MooseEnum>("component"))
{
}

ADReal
ADConductiveCurrent::computeQpResidual()
{
  // NOTE: In the future, need to add an AD capability to std::complex
  ADReal mu_omega_real = _mu_real[_qp] * _omega_real[_qp] - _mu_imag[_qp] * _omega_imag[_qp];
  ADReal mu_omega_imag = _mu_real[_qp] * _omega_imag[_qp] + _mu_imag[_qp] * _omega_real[_qp];

  ADRealVectorValue sigma_E_real = _cond_real[_qp] * _E_real[_qp] - _cond_imag[_qp] * _E_imag[_qp];
  ADRealVectorValue sigma_E_imag = _cond_imag[_qp] * _E_real[_qp] + _cond_real[_qp] * _E_imag[_qp];

  if (_component == EM::REAL)
  {
    return _test[_i][_qp] * -1.0 * (mu_omega_imag * sigma_E_real + mu_omega_real * sigma_E_imag);
  }
  else
  {
    return _test[_i][_qp] * (mu_omega_real * sigma_E_real - mu_omega_imag * sigma_E_imag);
  }
}
