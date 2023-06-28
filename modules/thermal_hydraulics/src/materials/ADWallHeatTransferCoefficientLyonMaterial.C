//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallHeatTransferCoefficientLyonMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADWallHeatTransferCoefficientLyonMaterial);

InputParameters
ADWallHeatTransferCoefficientLyonMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<MaterialPropertyName>("Hw",
                                        FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL,
                                        "Heat transfer coefficient material property");
  params.addParam<MaterialPropertyName>(
      "rho", FlowModelSinglePhase::DENSITY, "Density of the fluid");
  params.addParam<MaterialPropertyName>("vel", FlowModelSinglePhase::VELOCITY, "Fluid velocity");
  params.addParam<MaterialPropertyName>(
      "D_h", FlowModelSinglePhase::HYDRAULIC_DIAMETER, "Hydraulic diameter");
  params.addParam<MaterialPropertyName>(
      "cp", FlowModelSinglePhase::SPECIFIC_HEAT_CONSTANT_PRESSURE, "Specific heat of the fluid");
  params.addParam<MaterialPropertyName>(
      "mu", FlowModelSinglePhase::DYNAMIC_VISCOSITY, "Dynamic viscosity of the fluid");
  params.addParam<MaterialPropertyName>(
      "k", FlowModelSinglePhase::THERMAL_CONDUCTIVITY, "Heat conductivity of the fluid");
  params.addParam<MaterialPropertyName>(
      "T", FlowModelSinglePhase::TEMPERATURE, "Fluid temperature");
  params.addParam<MaterialPropertyName>("T_wall", FlowModel::TEMPERATURE_WALL, "Wall temperature");
  params.addClassDescription(
      "Computes wall heat transfer coefficient for liquid sodium using Lyon correlation");
  return params;
}

ADWallHeatTransferCoefficientLyonMaterial::ADWallHeatTransferCoefficientLyonMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _Hw(declareADProperty<Real>("Hw")),
    _rho(getADMaterialProperty<Real>("rho")),
    _vel(getADMaterialProperty<Real>("vel")),
    _D_h(getADMaterialProperty<Real>("D_h")),
    _k(getADMaterialProperty<Real>("k")),
    _mu(getADMaterialProperty<Real>("mu")),
    _cp(getADMaterialProperty<Real>("cp")),
    _T(getADMaterialProperty<Real>("T")),
    _T_wall(getADMaterialProperty<Real>("T_wall"))
{
}

void
ADWallHeatTransferCoefficientLyonMaterial::computeQpProperties()
{
  ADReal Pe = std::max(1.0, THM::Peclet(1., _cp[_qp], _rho[_qp], _vel[_qp], _D_h[_qp], _k[_qp]));
  ADReal Nu = 7 + 0.025 * std::pow(Pe, 0.8);
  _Hw[_qp] = THM::wallHeatTransferCoefficient(Nu, _k[_qp], _D_h[_qp]);
}
