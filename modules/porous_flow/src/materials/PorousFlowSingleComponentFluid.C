/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowSingleComponentFluid.h"

template<>
InputParameters validParams<PorousFlowSingleComponentFluid>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription("This Material calculates fluid properties at the quadpoints for a single component fluid");
  return params;
}

PorousFlowSingleComponentFluid::PorousFlowSingleComponentFluid(const InputParameters & parameters) :
    PorousFlowFluidPropertiesBase(parameters),

    _density_qp(declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + _phase)),
    _ddensity_qp_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _pressure_variable_name)),
    _ddensity_qp_dT(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _temperature_variable_name)),

    _internal_energy_qp(declareProperty<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase)),
    _dinternal_energy_qp_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase, _pressure_variable_name)),
    _dinternal_energy_qp_dT(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase, _temperature_variable_name)),

    _enthalpy_qp(declareProperty<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase)),
    _denthalpy_qp_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase, _pressure_variable_name)),
    _denthalpy_qp_dT(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase, _temperature_variable_name)),

    _fp(getUserObject<SinglePhaseFluidPropertiesPT>("fp"))
{
  _nodal_material = false;
}

void
PorousFlowSingleComponentFluid::computeQpProperties()
{
  // Density and derivatives wrt pressure and temperature at the qps
  Real Tk_qp = _temperature_qp[_qp] + _t_c2k;
  Real rho_qp, drho_dp_qp, drho_dT_qp;
  _fp.rho_dpT(_porepressure_qp[_qp][_phase_num], Tk_qp, rho_qp, drho_dp_qp, drho_dT_qp);
  _density_qp[_qp] = rho_qp;
  _ddensity_qp_dp[_qp] = drho_dp_qp;
  _ddensity_qp_dT[_qp] = drho_dT_qp;

  // Internal energy and derivatives wrt pressure and temperature at the qps
  Real e_qp, de_dp_qp, de_dT_qp;
  _fp.e_dpT(_porepressure_qp[_qp][_phase_num], Tk_qp, e_qp, de_dp_qp, de_dT_qp);
  _internal_energy_qp[_qp] = e_qp;
  _dinternal_energy_qp_dp[_qp] = de_dp_qp;
  _dinternal_energy_qp_dT[_qp] = de_dT_qp;

  // Enthalpy and derivatives wrt pressure and temperature at the qps
  Real h_qp, dh_dp_qp, dh_dT_qp;
  _fp.h_dpT(_porepressure_qp[_qp][_phase_num], Tk_qp, h_qp, dh_dp_qp, dh_dT_qp);
  _enthalpy_qp[_qp] = h_qp;
  _denthalpy_qp_dp[_qp] = dh_dp_qp;
  _denthalpy_qp_dT[_qp] = dh_dT_qp;
}
