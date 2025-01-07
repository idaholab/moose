//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallHeatTransferCoefficientMikityukFunctorMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADWallHeatTransferCoefficientMikityukFunctorMaterial);

InputParameters
ADWallHeatTransferCoefficientMikityukFunctorMaterial::validParams()
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
  params.addRequiredParam<Real>(
      "PoD", "The Pitch-to-diameter ratio value being assigned into the property");
  params.addClassDescription(
      "Computes wall heat transfer coefficient for liquid sodium using Mikityuk correlation");
  return params;
}

ADWallHeatTransferCoefficientMikityukFunctorMaterial::
    ADWallHeatTransferCoefficientMikityukFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _rho(getFunctor<ADReal>("rho")),
    _vel(getFunctor<ADReal>("vel")),
    _D_h(getFunctor<ADReal>("D_h")),
    _k(getFunctor<ADReal>("k")),
    _mu(getFunctor<ADReal>("mu")),
    _cp(getFunctor<ADReal>("cp")),
    _T(getFunctor<ADReal>("T")),
    _T_wall(getFunctor<ADReal>("T_wall")),
    _PoD(getParam<Real>("PoD"))
{
  addFunctorProperty<ADReal>(
      "Hw",
      [this](const auto & r, const auto & t) -> ADReal
      {
        ADReal Pe =
            std::max(1.0, THM::Peclet(1., _cp(r, t), _rho(r, t), _vel(r, t), _D_h(r, t), _k(r, t)));

        if (_PoD > 1.5 || _PoD < 1.1 || Pe > 5000 || Pe < 30)
        {
          mooseDoOnce(
              mooseWarning("Mikityuk's correlation is valid when Pe is between 30 and 5000, and "
                           "P/D is between 1.1 "
                           "and 1.5. Be aware that using values out of this range may lead to "
                           "significant errors in your results!"));
        }

        ADReal Nu = 0.047 * (1 - std::exp(-3.8 * (_PoD - 1))) * (std::pow(Pe, 0.77) + 250);
        return THM::wallHeatTransferCoefficient(Nu, _k(r, t), _D_h(r, t));
      });
}
