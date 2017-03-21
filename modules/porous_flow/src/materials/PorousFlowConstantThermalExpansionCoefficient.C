/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowConstantThermalExpansionCoefficient.h"

template <>
InputParameters
validParams<PorousFlowConstantThermalExpansionCoefficient>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
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
      "rock without flulid, or with a fluid that is free to move in and out of the rock)");
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
