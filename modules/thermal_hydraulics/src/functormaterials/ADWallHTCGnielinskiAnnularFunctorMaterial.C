//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallHTCGnielinskiAnnularFunctorMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADWallHTCGnielinskiAnnularFunctorMaterial);

InputParameters
ADWallHTCGnielinskiAnnularFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addParam<MooseFunctorName>("Hw",
                                    FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL,
                                    "Name to give the heat transfer coefficient material property");
  params.addParam<MooseFunctorName>(
      "rho", FlowModelSinglePhase::DENSITY, "Fluid density material property");
  params.addParam<MooseFunctorName>(
      "vel", FlowModelSinglePhase::VELOCITY, "Fluid velocity material property");
  params.addParam<MooseFunctorName>("cp",
                                    FlowModelSinglePhase::SPECIFIC_HEAT_CONSTANT_PRESSURE,
                                    "Fluid isobaric specific heat capacity material property");
  params.addParam<MooseFunctorName>(
      "mu", FlowModelSinglePhase::DYNAMIC_VISCOSITY, "Fluid dynamic viscosity material property");
  params.addParam<MooseFunctorName>("k",
                                    FlowModelSinglePhase::THERMAL_CONDUCTIVITY,
                                    "Fluid thermal conductivity material property");
  params.addParam<MooseFunctorName>(
      "p", FlowModelSinglePhase::PRESSURE, "Fluid pressure material property");
  params.addParam<MooseFunctorName>(
      "T", FlowModelSinglePhase::TEMPERATURE, "Fluid temperature material property");
  params.addParam<MooseFunctorName>(
      "T_wall", FlowModel::TEMPERATURE_WALL, "Wall temperature material property");
  params.addRequiredParam<Real>("D_inner", "Inner diameter of the annulus [m]");
  params.addRequiredParam<Real>("D_outer", "Outer diameter of the annulus [m]");
  params.addRequiredParam<Real>("channel_length", "Channel length [m]");
  params.addRequiredParam<bool>("at_inner_wall", "True if heat transfer is at inner wall");
  params.addRequiredParam<bool>("fluid_is_gas", "True if the fluid is a gas");
  params.addParam<Real>("gas_heating_correction_exponent",
                        0,
                        "Exponent for the ratio of bulk fluid temperature to wall temperature for "
                        "the Nusselt number correction factor when heating a gas");
  params.addRequiredParam<UserObjectName>("fluid_properties", "Fluid properties object");

  params.addClassDescription("Computes wall heat transfer coefficient for gases and water in an "
                             "annular flow channel using the Gnielinski correlation");

  return params;
}

ADWallHTCGnielinskiAnnularFunctorMaterial::ADWallHTCGnielinskiAnnularFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _rho(getFunctor<ADReal>("rho")),
    _vel(getFunctor<ADReal>("vel")),
    _k(getFunctor<ADReal>("k")),
    _mu(getFunctor<ADReal>("mu")),
    _cp(getFunctor<ADReal>("cp")),
    _p(getFunctor<ADReal>("p")),
    _T(getFunctor<ADReal>("T")),
    _T_wall(getFunctor<ADReal>("T_wall")),
    _D_inner(getParam<Real>("D_inner")),
    _D_outer(getParam<Real>("D_outer")),
    _D_h(_D_outer - _D_inner),
    _a(_D_inner / _D_outer),
    _L(getParam<Real>("channel_length")),
    _at_inner_wall(getParam<bool>("at_inner_wall")),
    _fluid_is_gas(getParam<bool>("fluid_is_gas")),
    _n(getParam<Real>("gas_heating_correction_exponent")),
    _provided_gas_heating_correction_exponent(isParamSetByUser("gas_heating_correction_exponent")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
  addFunctorProperty<ADReal>(
      "Hw",
      [this](const auto & r, const auto & t) -> ADReal
      {
        const ADReal Pr = THM::Prandtl(_cp(r, t), _mu(r, t), _k(r, t));

        ADReal K;
        if (_fluid_is_gas)
        {
          if (_T_wall(r, t) > _T(r, t) && !_provided_gas_heating_correction_exponent)
            mooseError(
                "If wall temperature ever exceeds the fluid temperature, even in iteration, and "
                "the "
                "fluid is a gas, then the parameter 'gas_heating_correction_exponent' must be "
                "provided.");
          K = std::pow(_T(r, t) / _T_wall(r, t), _n);
        }
        else
        {
          const ADReal cp_wall = _fp.cp_from_p_T(_p(r, t), _T_wall(r, t));
          const ADReal mu_wall = _fp.mu_from_p_T(_p(r, t), _T_wall(r, t));
          const ADReal k_wall = _fp.k_from_p_T(_p(r, t), _T_wall(r, t));
          const ADReal Pr_wall = THM::Prandtl(cp_wall, mu_wall, k_wall);
          K = std::pow(Pr / Pr_wall, 0.11);
        }

        ADReal Re = THM::Reynolds(1.0, _rho(r, t), _vel(r, t), _D_h, _mu(r, t));
        if (Re < 1e4)
        {
          mooseDoOnce(mooseWarning(
              "This material uses a correlation that is valid for Re > 1e4, but the "
              "material was evaluated with an Re = " +
              Moose::stringify(MetaPhysicL::raw_value(Re)) +
              ". This and all subsequent evaluations will be given a lower bound of 1e4."));
          Re = 1e4;
        }
        const ADReal Re_star = Re * ((1 + _a * _a) * std::log(_a) + (1 - _a * _a)) /
                               (std::pow(1 - _a, 2) * std::log(_a));

        const ADReal f_ann = std::pow(1.8 * std::log10(Re_star) - 1.5, -2.0);
        const ADReal k1 = 1.07 + 900.0 / Re - 0.63 / (1.0 + 10.0 * Pr);

        ADReal F_ann;
        if (_at_inner_wall)
          F_ann = 0.75 * std::pow(_a, -0.17);
        else
          F_ann = 0.9 - 0.15 * std::pow(_a, 0.6);

        const ADReal Nu = f_ann / 8.0 * Re * Pr /
                          (k1 + 12.7 * std::sqrt(f_ann / 8.0) * (std::pow(Pr, 2.0 / 3.0) - 1.0)) *
                          (1.0 + std::pow(_D_h / _L, 2.0 / 3.0)) * F_ann * K;

        return THM::wallHeatTransferCoefficient(Nu, _k(r, t), _D_h);
      });
}
