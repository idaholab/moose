//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallHeatTransferCoefficientGnielinskiFunctorMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADWallHeatTransferCoefficientGnielinskiFunctorMaterial);

InputParameters
ADWallHeatTransferCoefficientGnielinskiFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addParam<MooseFunctorName>("Hw",
                                    FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL,
                                    "Heat transfer coefficient material property");
  params.addParam<MooseFunctorName>("rho", FlowModelSinglePhase::DENSITY, "Density of the fluid");
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
  params.addClassDescription("Computes wall heat transfer coefficient for gases and water using "
                             "the Gnielinski correlation");
  return params;
}

ADWallHeatTransferCoefficientGnielinskiFunctorMaterial::
    ADWallHeatTransferCoefficientGnielinskiFunctorMaterial(const InputParameters & parameters)
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
        return THM::wallHeatTransferCoefficient(Nu, _k(r, t), _D_h(r, t));
      });
}
