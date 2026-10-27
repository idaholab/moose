//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDittusBoelterHTCFunctorMaterial.h"

#include "Numerics.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", ADDittusBoelterHTCFunctorMaterial);

InputParameters
ADDittusBoelterHTCFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription(
      "Computes the wall heat transfer coefficient of a path-integrated flow segment using "
      "the Dittus-Boelter correlation, as a functor material property.");
  params.addParam<MooseFunctorName>(
      "htc_name",
      "Name of the heat transfer coefficient functor property. Defaults to the name of this "
      "object.");
  params.addCoupledVar(
      "mass_flow_rate", {}, "Mass flow rate through the segment. Takes a scalar variable name");
  params.addCoupledVar(
      "T_fluid", {}, "Fluid temperature of the segment. Takes a scalar variable name");
  params.addCoupledVar(
      "T_fluid_inlet",
      {},
      "Fluid temperature of the nominal inlet segment/component. Takes a scalar variable name");
  params.addCoupledVar(
      "T_fluid_outlet",
      {},
      "Fluid temperature of the nominal outlet segment/component. Takes a scalar variable name");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");
  params.addParam<UserObjectName>("reference_fp",
                                  "Liquid-phase fluid properties; see momentum kernel.");
  params.addRequiredParam<MooseFunctorName>("reference_pressure", "System reference pressure [Pa]");
  params.addRequiredParam<MooseFunctorName>("flow_area",
                                            "Cross-sectional area of the fluid flow channel [m^2]");
  params.addRequiredParam<MooseFunctorName>("wetted_perimeter",
                                            "Wetted perimeter of the fluid flow channel [m]");

  return params;
}

ADDittusBoelterHTCFunctorMaterial::ADDittusBoelterHTCFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties")),
    _fp_ref(isParamValid("reference_fp") ? &getUserObjectByName<SinglePhaseFluidProperties>(
                                               getParam<UserObjectName>("reference_fp"))
                                         : nullptr),
    _m(ScalarCoupleable::adCoupledScalarValue("mass_flow_rate")),
    _T_fluid(ScalarCoupleable::adCoupledScalarValue("T_fluid")),
    _T_fluid_inlet(ScalarCoupleable::adCoupledScalarValue("T_fluid_inlet")),
    _T_fluid_outlet(ScalarCoupleable::adCoupledScalarValue("T_fluid_outlet")),
    _P_ref(getFunctor<ADReal>("reference_pressure")),
    _flow_area(getFunctor<ADReal>("flow_area")),
    _wetted_perimeter(getFunctor<ADReal>("wetted_perimeter"))
{
  const MooseFunctorName htc_name =
      isParamValid("htc_name") ? getParam<MooseFunctorName>("htc_name") : MooseFunctorName(name());
  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());

  addFunctorProperty<ADReal>(
      htc_name,
      [this](const auto & r, const auto & t) -> ADReal
      {
        using std::abs, std::pow;

        // Scalar variables have a single component
        const unsigned int i = 0;

        // Upwind the segment inlet temperature on the flow direction
        const auto T_in =
            1.0 / 2.0 * (1 - _m[i] / sqrt(_m[i] * _m[i] + 1.0e-08)) * _T_fluid_outlet[i] +
            1.0 / 2.0 * (1 + _m[i] / sqrt(_m[i] * _m[i] + 1.0e-08)) * _T_fluid_inlet[i];

        // Properties at the bulk film temperature of the segment
        const auto T_film = (_T_fluid[i] + T_in) / 2;
        const auto P_ref = _P_ref(r, t);
        const auto mu = _fp.mu_from_p_T(P_ref, T_film);
        const auto cp = _fp.cp_from_p_T(P_ref, T_film);
        const auto k = _fp.k_from_p_T(P_ref, T_film);
        const auto rho = _fp.rho_from_p_T(P_ref, T_film);
        const auto rho_ref = _fp_ref ? _fp_ref->rho_from_p_T(P_ref, T_film) : rho;

        const auto A = _flow_area(r, t);
        const auto D_h = 4.0 * A / _wetted_perimeter(r, t);
        // Formed from the mass flux G = rho * vel, which the segment's mass flow rate gives
        // directly, so no density evaluation is needed
        const auto G = abs(_m[i]) / A * rho / rho_ref;
        const auto Re = max(G * D_h / mu, 1.0e-03);
        const auto Pr = THM::Prandtl(cp, mu, k);
        const auto Nu = max(0.023 * pow(Re, 0.8) * pow(Pr, 0.4), 4.0);

        return THM::wallHeatTransferCoefficient(Nu, k, D_h);
      },
      clearance_schedule);
}
