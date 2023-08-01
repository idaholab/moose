//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallHeatTransferCoefficientGnielinskiMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADWallHeatTransferCoefficientGnielinskiMaterial);

InputParameters
ADWallHeatTransferCoefficientGnielinskiMaterial::validParams()
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
  params.addClassDescription("Computes wall heat transfer coefficient for gases and water using "
                             "the Gnielinski correlation");
  return params;
}

ADWallHeatTransferCoefficientGnielinskiMaterial::ADWallHeatTransferCoefficientGnielinskiMaterial(
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
ADWallHeatTransferCoefficientGnielinskiMaterial::computeQpProperties()
{
  ADReal Pr = THM::Prandtl(_cp[_qp], _mu[_qp], _k[_qp]);
  ADReal Re = std::max(1.0, THM::Reynolds(1., _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]));

  if (Re < 2300 || Re > 5E+6 || Pr < 0.5 || Pr > 2000)
  {
    mooseDoOnce(mooseWarning(
        "The Gnielinski correlation is valid when Pr is between 0.5 and 2000, and Re is "
        "between 2300 and 5000000.  Be aware that using values out of this range may lead to "
        "significant errors in your results!"));
  }

  ADReal f = std::pow(1.82 * std::log10(Re) - 1.64, -2.0);
  ADReal Nu = ((f / 8.0) * std::max(0.0, Re - 1000.0) * Pr) /
              (1.0 + 12.7 * std::sqrt(f / 8.0) * (std::pow(Pr, 2.0 / 3.0) - 1.0));
  _Hw[_qp] = THM::wallHeatTransferCoefficient(Nu, _k[_qp], _D_h[_qp]);
}
