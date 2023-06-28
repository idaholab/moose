//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallHeatTransferCoefficientSchadMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADWallHeatTransferCoefficientSchadMaterial);

InputParameters
ADWallHeatTransferCoefficientSchadMaterial::validParams()
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
  params.addRequiredParam<Real>(
      "PoD", "The Pitch-to-diameter ratio value being assigned into the property");
  params.addClassDescription(
      "Computes wall heat transfer coefficient for liquid sodium using Schad-modified correlation");
  return params;
}

ADWallHeatTransferCoefficientSchadMaterial::ADWallHeatTransferCoefficientSchadMaterial(
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
    _T_wall(getADMaterialProperty<Real>("T_wall")),
    _PoD(getParam<Real>("PoD"))
{
}

void
ADWallHeatTransferCoefficientSchadMaterial::computeQpProperties()
{
  ADReal Pe = std::max(1.0, THM::Peclet(1., _cp[_qp], _rho[_qp], _vel[_qp], _D_h[_qp], _k[_qp]));

  if (_PoD > 1.5 || _PoD < 1.1 || Pe > 1000)
  {
    mooseDoOnce(mooseWarning("Schad's correlation is valid when Pe<1000, and P/D is between 1.1 "
                             "and 1.5. Be aware that using values out of this range may lead to "
                             "significant errors in your results!"));
  }

  if (Pe < 150)
  {
    ADReal Nu = 4.496 * (-16.15 + 24.96 * _PoD - 8.55 * std::pow(_PoD, 2));
    _Hw[_qp] = THM::wallHeatTransferCoefficient(Nu, _k[_qp], _D_h[_qp]);
  }
  else
  {
    ADReal Nu = (-16.15 + 24.96 * _PoD - 8.55 * std::pow(_PoD, 2)) * std::pow(Pe, 0.3);
    _Hw[_qp] = THM::wallHeatTransferCoefficient(Nu, _k[_qp], _D_h[_qp]);
  }
}
