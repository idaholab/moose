//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowConstantThermalExpansionCoefficient.h"

registerMooseObject("PorousFlowApp", PorousFlowConstantThermalExpansionCoefficient);

InputParameters
PorousFlowConstantThermalExpansionCoefficient::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.addRangeCheckedParam<Real>(
      "biot_coefficient", 1.0, "biot_coefficient>=0 & biot_coefficient<=1", "Biot coefficient");
  params.addRangeCheckedParam<Real>("fluid_coefficient",
                                    2.1E-4,
                                    "fluid_coefficient>=0",
                                    "Volumetric coefficient of thermal expansion for the fluid");
  params.addRequiredRangeCheckedParam<Real>(
      "drained_coefficient",
      "drained_coefficient>=0.0",
      "Volumetric coefficient of thermal expansion of the drained porous skeleton (ie the porous "
      "rock without fluid, or with a fluid that is free to move in and out of the rock)");
  params.addPrivateParam<std::string>("pf_material_type", "thermal_expansion");
  params.addClassDescription("Computes the effective thermal expansion coefficient, (biot_coeff - "
                             "porosity) * drained_coefficient + porosity * fluid_coefficient.");
  return params;
}

PorousFlowConstantThermalExpansionCoefficient::PorousFlowConstantThermalExpansionCoefficient(
    const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _biot_coefficient(getParam<Real>("biot_coefficient")),
    _fluid_coefficient(getParam<Real>("fluid_coefficient")),
    _drained_coefficient(getParam<Real>("drained_coefficient")),
    _porosity(_nodal_material ? getMaterialProperty<Real>("PorousFlow_porosity_nodal")
                              : getMaterialProperty<Real>("PorousFlow_porosity_qp")),
    _coeff(_nodal_material
               ? declareProperty<Real>("PorousFlow_constant_thermal_expansion_coefficient_nodal")
               : declareProperty<Real>("PorousFlow_constant_thermal_expansion_coefficient_qp")),
    _coeff_old(_nodal_material ? getMaterialPropertyOld<Real>(
                                     "PorousFlow_constant_thermal_expansion_coefficient_nodal")
                               : getMaterialPropertyOld<Real>(
                                     "PorousFlow_constant_thermal_expansion_coefficient_qp"))
{
}

void
PorousFlowConstantThermalExpansionCoefficient::initQpStatefulProperties()
{
  _coeff[_qp] = (_biot_coefficient - _porosity[_qp]) * _drained_coefficient +
                _porosity[_qp] * _fluid_coefficient;
}

void
PorousFlowConstantThermalExpansionCoefficient::computeQpProperties()
{
  _coeff[_qp] = _coeff_old[_qp];
}
