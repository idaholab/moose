//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSensitivity.h"

registerMooseObject("HeatTransferApp", ThermalSensitivity);

InputParameters
ThermalSensitivity::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes cost sensitivity needed for multimaterial SIMP method.");
  params.addRequiredCoupledVar("design_density", "Design density variable name.");
  params.addRequiredCoupledVar("temperature", "temperature");
  params.addRequiredParam<MaterialPropertyName>("thermal_conductivity",
                                                "DerivativeParsedMaterial for cost of materials.");
  return params;
}

ThermalSensitivity::ThermalSensitivity(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _sensitivity(declareProperty<Real>(_base_name + "thermal_sensitivity")),
    _design_density(coupledValue("design_density")),
    _design_density_name(coupledName("design_density", 0)),
    _grad_temperature(coupledGradient("temperature")),
    _thermal_conductivity(
        getMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("thermal_conductivity"))),
    _dTdp(getMaterialPropertyDerivativeByName<Real>(
        getParam<MaterialPropertyName>("thermal_conductivity"), _design_density_name))
{
}

void
ThermalSensitivity::computeQpProperties()
{
  const Real thermal_compliance =
      0.5 * _thermal_conductivity[_qp] * _grad_temperature[_qp] * _grad_temperature[_qp];
  _sensitivity[_qp] = -_dTdp[_qp] * thermal_compliance / _thermal_conductivity[_qp];
}
