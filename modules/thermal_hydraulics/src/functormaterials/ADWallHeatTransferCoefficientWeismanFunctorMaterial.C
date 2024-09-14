//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallHeatTransferCoefficientWeismanFunctorMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADWallHeatTransferCoefficientWeismanFunctorMaterial);

InputParameters
ADWallHeatTransferCoefficientWeismanFunctorMaterial::validParams()
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
  MooseEnum bundle_array("SQUARE TRIANGULAR", "SQUARE");
  params.addParam<MooseEnum>("bundle_array", bundle_array, "The type of the rod bundle array");
  params.addClassDescription(
      "Computes wall heat transfer coefficient for water using the Weisman correlation");
  return params;
}

ADWallHeatTransferCoefficientWeismanFunctorMaterial::
    ADWallHeatTransferCoefficientWeismanFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _rho(getFunctor<ADReal>("rho")),
    _vel(getFunctor<ADReal>("vel")),
    _D_h(getFunctor<ADReal>("D_h")),
    _k(getFunctor<ADReal>("k")),
    _mu(getFunctor<ADReal>("mu")),
    _cp(getFunctor<ADReal>("cp")),
    _T(getFunctor<ADReal>("T")),
    _T_wall(getFunctor<ADReal>("T_wall")),
    _PoD(getParam<Real>("PoD")),
    _bundle_array(getParam<MooseEnum>("bundle_array").getEnum<Bundle_array>())
{
  addFunctorProperty<ADReal>(
      "Hw",
      [this](const auto & r, const auto & t) -> ADReal
      {
        switch (_bundle_array)
        {
          case Bundle_array::SQUARE:
          {
            if (_PoD > 1.3 || _PoD < 1.1)
              mooseDoOnce(mooseWarning(
                  "The Weisman correlation for square arrays is valid when P/D is between 1.1 "
                  "and 1.3. Be aware that using values out of this range may lead to "
                  "significant errors in your results!"));
            ADReal Pr = THM::Prandtl(_cp(r, t), _mu(r, t), _k(r, t));
            ADReal Re =
                std::max(1.0, THM::Reynolds(1., _rho(r, t), _vel(r, t), _D_h(r, t), _mu(r, t)));
            ADReal n = (_T(r, t) < _T_wall(r, t)) ? 0.4 : 0.3;
            ADReal Nu = 0.023 * std::pow(Re, 4. / 5.) * std::pow(Pr, n) * (1.826 * _PoD - 1.0430);
            return THM::wallHeatTransferCoefficient(Nu, _k(r, t), _D_h(r, t));
            break;
          }
          case Bundle_array::TRIANGULAR:
          {
            if (_PoD > 1.5 || _PoD < 1.1)
              mooseDoOnce(mooseWarning(
                  "The Weisman correlation for triangular arrays is valid when P/D is between 1.1 "
                  "and 1.5. Be aware that using values out of this range may lead to "
                  "significant errors in your results!"));
            ADReal Pr = THM::Prandtl(_cp(r, t), _mu(r, t), _k(r, t));
            ADReal Re =
                std::max(1.0, THM::Reynolds(1., _rho(r, t), _vel(r, t), _D_h(r, t), _mu(r, t)));
            ADReal n = (_T(r, t) < _T_wall(r, t)) ? 0.4 : 0.3;
            ADReal Nu = 0.023 * std::pow(Re, 4. / 5.) * std::pow(Pr, n) * (1.130 * _PoD - 0.2609);
            return THM::wallHeatTransferCoefficient(Nu, _k(r, t), _D_h(r, t));
            break;
          }
          default:
            mooseError("Invalid 'bundle_array' parameter.");
        }
      });
}
