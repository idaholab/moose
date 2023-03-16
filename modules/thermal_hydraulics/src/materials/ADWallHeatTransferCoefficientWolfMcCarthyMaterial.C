//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallHeatTransferCoefficientWolfMcCarthyMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADWallHeatTransferCoefficientWolfMcCarthyMaterial);

InputParameters
ADWallHeatTransferCoefficientWolfMcCarthyMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<MaterialPropertyName>("Hw",
                                        FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL,
                                        "Heat transfer coefficient material property");
  params.addParam<MaterialPropertyName>(
      "rho", FlowModelSinglePhase::DENSITY, "Density of the liquid");
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
      "Computes wall heat transfer coefficient using Wolf-McCarthy correlation");
  return params;
}

ADWallHeatTransferCoefficientWolfMcCarthyMaterial::
    ADWallHeatTransferCoefficientWolfMcCarthyMaterial(const InputParameters & parameters)
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
ADWallHeatTransferCoefficientWolfMcCarthyMaterial::computeQpProperties()
{
  ADReal Pr = THM::Prandtl(_cp[_qp], _mu[_qp], _k[_qp]);
  ADReal Re = std::max(1.0, THM::Reynolds(1., _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]));
  ADReal Nu = 0.025 * std::pow(Re, 0.8) * std::pow(Pr, 0.4) *
              std::pow(std::max(_T[_qp], _T_wall[_qp]) / _T[_qp], -0.55);

  _Hw[_qp] = THM::wallHeatTransferCoefficient(Nu, _k[_qp], _D_h[_qp]);
}
