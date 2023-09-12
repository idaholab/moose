//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalCompliance.h"

registerMooseObject("troutApp", ThermalCompliance);

InputParameters
ThermalCompliance::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes cost sensitivity needed for multimaterial SIMP method.");
  params.addRequiredCoupledVar("temperature", "temperature");
  params.addRequiredParam<MaterialPropertyName>("thermal_conductivity",
                                                "DerivativeParsedMaterial for cost of materials.");
  return params;
}

ThermalCompliance::ThermalCompliance(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _grad_temperature(coupledGradient("temperature")),
    _thermal_conductivity(
        getMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("thermal_conductivity"))),
    _thermal_compliance(declareProperty<Real>("thermal_compliance"))
{
}

void
ThermalCompliance::computeQpProperties()
{
  const Real thermal_compliance =
      0.5 * _thermal_conductivity[_qp] * _grad_temperature[_qp] * _grad_temperature[_qp];
  _thermal_compliance[_qp] = thermal_compliance;

  // C_e (rho_e) = A_c rho^{1/p} + B_c; A_c = (C_i - C_{i+1})/(rho_i^{1/p} - rho_{i+1}^{1/p}); B_c =
  // C_i - A_c rho_i^{1/p}
}
