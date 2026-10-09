//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosThermalSensitivity.h"

registerKokkosMaterial("HeatTransferApp", KokkosThermalSensitivity);

InputParameters
KokkosThermalSensitivity::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes cost sensitivity needed for multimaterial SIMP method.");
  params.addRequiredCoupledVar("design_density", "Design density variable name.");
  params.addRequiredCoupledVar("temperature", "temperature");
  params.addRequiredParam<MaterialPropertyName>("thermal_conductivity",
                                                "DerivativeParsedMaterial for cost of materials.");
  return params;
}

KokkosThermalSensitivity::KokkosThermalSensitivity(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _sensitivity(declareKokkosPropertyByName<Real>(_base_name + "thermal_sensitivity")),
    _grad_temperature(kokkosCoupledGradient("temperature")),
    _thermal_conductivity(getKokkosMaterialPropertyByName<Real>(
        getParam<MaterialPropertyName>("thermal_conductivity"))),
    _dTdp(getKokkosMaterialPropertyDerivativeByName<Real>(
        getParam<MaterialPropertyName>("thermal_conductivity"), {coupledName("design_density", 0)}))
{
}
