//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallHeatTransferCoefficientWolfMcCarthyFunctorMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp",
                    ADWallHeatTransferCoefficientWolfMcCarthyFunctorMaterial);

InputParameters
ADWallHeatTransferCoefficientWolfMcCarthyFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addParam<MooseFunctorName>("Hw",
                                    FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL,
                                    "Heat transfer coefficient material property");
  params.addParam<MooseFunctorName>("rho", FlowModelSinglePhase::DENSITY, "Density of the liquid");
  params.addParam<MooseFunctorName>("vel", FlowModelSinglePhase::VELOCITY, "Fluid velocity");
  params.addParam<MooseFunctorName>(
      "D_h", FlowModelSinglePhase::HYDRAULIC_DIAMETER, "Hydraulic diameter");
  params.addParam<MooseFunctorName>(
      "cp", FlowModelSinglePhase::SPECIFIC_HEAT_CONSTANT_PRESSURE, "Specific heat of the fluid");
  params.addParam<MooseFunctorName>(
      "mu", FlowModelSinglePhase::DYNAMIC_VISCOSITY, "Dynamic viscosity of the fluid");
  params.addParam<MooseFunctorName>(
      "k", FlowModelSinglePhase::THERMAL_CONDUCTIVITY, "Heat conductivity of the fluid");
  params.addParam<MooseFunctorName>("T", FlowModelSinglePhase::TEMPERATURE, "Fluid temperature");
  params.addParam<MooseFunctorName>("T_wall", FlowModel::TEMPERATURE_WALL, "Wall temperature");
  params.addClassDescription(
      "Computes wall heat transfer coefficient using Wolf-McCarthy correlation");
  return params;
}

ADWallHeatTransferCoefficientWolfMcCarthyFunctorMaterial::
    ADWallHeatTransferCoefficientWolfMcCarthyFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _rho(getFunctor<ADReal>("rho")),
    _vel(getFunctor<ADReal>("vel")),
    _D_h(getFunctor<ADReal>("D_h")),
    _k(getFunctor<ADReal>("k")),
    _mu(getFunctor<ADReal>("mu")),
    _cp(getFunctor<ADReal>("cp")),
    _T(getFunctor<ADReal>("T")),
    _T_wall(getFunctor<ADReal>("T_wall"))
{
  addFunctorProperty<ADReal>(
      "Hw",
      [this](const auto & r, const auto & t) -> ADReal
      {
        ADReal Pr = THM::Prandtl(_cp(r, t), _mu(r, t), _k(r, t));
        ADReal Re = std::max(1.0, THM::Reynolds(1., _rho(r, t), _vel(r, t), _D_h(r, t), _mu(r, t)));
        ADReal Nu = 0.025 * std::pow(Re, 0.8) * std::pow(Pr, 0.4) *
                    std::pow(std::max(_T(r, t), _T_wall(r, t)) / _T(r, t), -0.55);

        return THM::wallHeatTransferCoefficient(Nu, _k(r, t), _D_h(r, t));
      });
}
