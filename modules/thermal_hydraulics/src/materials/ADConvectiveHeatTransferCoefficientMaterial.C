//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConvectiveHeatTransferCoefficientMaterial.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADConvectiveHeatTransferCoefficientMaterial);

InputParameters
ADConvectiveHeatTransferCoefficientMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<MaterialPropertyName>("Hw",
                                        FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL,
                                        "Heat transfer coefficient material property");
  params.addRequiredParam<MaterialPropertyName>("Nu", "Nusselt number");
  params.addParam<MaterialPropertyName>(
      "D_h", FlowModelSinglePhase::HYDRAULIC_DIAMETER, "Hydraulic diameter");
  params.addParam<MaterialPropertyName>(
      "k", FlowModelSinglePhase::THERMAL_CONDUCTIVITY, "Thermal conductivity");
  params.addClassDescription("Computes convective heat transfer coefficient from Nusselt number");
  return params;
}

ADConvectiveHeatTransferCoefficientMaterial::ADConvectiveHeatTransferCoefficientMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _Hw(declareADProperty<Real>("Hw")),
    _Nu(getADMaterialProperty<Real>("Nu")),
    _D_h(getADMaterialProperty<Real>("D_h")),
    _k(getADMaterialProperty<Real>("k"))
{
}

void
ADConvectiveHeatTransferCoefficientMaterial::computeQpProperties()
{
  _Hw[_qp] = THM::wallHeatTransferCoefficient(_Nu[_qp], _k[_qp], _D_h[_qp]);
}
