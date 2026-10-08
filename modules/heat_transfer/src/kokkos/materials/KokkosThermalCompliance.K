//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosThermalCompliance.h"

registerKokkosMaterial("HeatTransferApp", KokkosThermalCompliance);

InputParameters
KokkosThermalCompliance::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes cost sensitivity needed for multimaterial SIMP method.");
  params.addRequiredCoupledVar("temperature", "temperature");
  params.addRequiredParam<MaterialPropertyName>("thermal_conductivity",
                                                "DerivativeParsedMaterial for cost of materials.");
  return params;
}

KokkosThermalCompliance::KokkosThermalCompliance(const InputParameters & parameters)
  : Material(parameters),
    _grad_temperature(kokkosCoupledGradient("temperature")),
    _thermal_conductivity(getKokkosMaterialPropertyByName<Real>(
        getParam<MaterialPropertyName>("thermal_conductivity"))),
    _thermal_compliance(declareKokkosPropertyByName<Real>("thermal_compliance"))
{
}
