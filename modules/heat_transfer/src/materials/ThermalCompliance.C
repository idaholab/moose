//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalCompliance.h"

registerMooseObject("HeatTransferApp", ThermalCompliance);

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
  _thermal_compliance[_qp] =
      0.5 * _thermal_conductivity[_qp] * _grad_temperature[_qp] * _grad_temperature[_qp];
}
