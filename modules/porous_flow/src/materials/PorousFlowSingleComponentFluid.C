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
  params.addClassDescription("This Material calculates fluid properties for a single component fluid");
  return params;
}

PorousFlowSingleComponentFluid::PorousFlowSingleComponentFluid(const InputParameters & parameters) :
    PorousFlowFluidPropertiesBase(parameters),

    _density_nodal(declareProperty<Real>("PorousFlow_fluid_phase_density" + _phase)),
    _density_nodal_old(declarePropertyOld<Real>("PorousFlow_fluid_phase_density" + _phase)),
    _ddensity_nodal_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + _phase, _pressure_variable_name)),
    _ddensity_nodal_dT(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + _phase, _temperature_variable_name)),
    _density_qp(declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + _phase)),
    _ddensity_qp_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _pressure_variable_name)),
    _ddensity_qp_dT(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _temperature_variable_name)),

    _viscosity_nodal(declareProperty<Real>("PorousFlow_viscosity" + _phase)),
    _dviscosity_nodal_dp(declarePropertyDerivative<Real>("PorousFlow_viscosity" + _phase, _pressure_variable_name)),
    _dviscosity_nodal_dT(declarePropertyDerivative<Real>("PorousFlow_viscosity" + _phase, _temperature_variable_name)),

    _internal_energy_nodal(declareProperty<Real>("PorousFlow_fluid_phase_internal_energy_nodal" + _phase)),
    _internal_energy_nodal_old(declarePropertyOld<Real>("PorousFlow_fluid_phase_internal_energy_nodal" + _phase)),
    _dinternal_energy_nodal_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_nodal" + _phase, _pressure_variable_name)),
    _dinternal_energy_nodal_dT(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_nodal" + _phase, _temperature_variable_name)),
    _internal_energy_qp(declareProperty<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase)),
    _dinternal_energy_qp_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase, _pressure_variable_name)),
    _dinternal_energy_qp_dT(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase, _temperature_variable_name)),

    _enthalpy_nodal(declareProperty<Real>("PorousFlow_fluid_phase_enthalpy_nodal" + _phase)),
    _enthalpy_nodal_old(declarePropertyOld<Real>("PorousFlow_fluid_phase_enthalpy_nodal" + _phase)),
    _denthalpy_nodal_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_nodal" + _phase, _pressure_variable_name)),
    _denthalpy_nodal_dT(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_nodal" + _phase, _temperature_variable_name)),
    _enthalpy_qp(declareProperty<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase)),
    _denthalpy_qp_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase, _pressure_variable_name)),
    _denthalpy_qp_dT(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase, _temperature_variable_name)),

    _fp(getUserObject<SinglePhaseFluidPropertiesPT>("fp"))
{
}

void
PorousFlowSingleComponentFluid::initQpStatefulProperties()
{
  _density_nodal[_qp] = _fp.rho(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp] + _t_c2k);
  _internal_energy_nodal[_qp] = _fp.e(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp]  + _t_c2k);
  _enthalpy_nodal[_qp] = _fp.h(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp]  + _t_c2k);
}

void
PorousFlowSingleComponentFluid::computeQpProperties()
{
  // Density and derivatives wrt pressure and temperature at the nodes
  Real Tk_nodal = _temperature_nodal[_qp] + _t_c2k;
  Real rho_nodal, drho_dp_nodal, drho_dT_nodal;
  _fp.rho_dpT(_porepressure_nodal[_qp][_phase_num], Tk_nodal, rho_nodal, drho_dp_nodal, drho_dT_nodal);
  _density_nodal[_qp] = rho_nodal;
  _ddensity_nodal_dp[_qp] = drho_dp_nodal;
  _ddensity_nodal_dT[_qp] = drho_dT_nodal;

  // Density and derivatives wrt pressure and temperature at the qps
  Real Tk_qp = _temperature_qp[_qp] + _t_c2k;
  Real rho_qp, drho_dp_qp, drho_dT_qp;
  _fp.rho_dpT(_porepressure_qp[_qp][_phase_num], Tk_qp, rho_qp, drho_dp_qp, drho_dT_qp);
  _density_qp[_qp] = rho_qp;
  _ddensity_qp_dp[_qp] = drho_dp_qp;
  _ddensity_qp_dT[_qp] = drho_dT_qp;

  // Viscosity and derivatives wrt pressure and temperature at the nodes.
  // Note that dmu_dp = dmu_drho * drho_dp
  Real mu, dmu_drho, dmu_dT;
  _fp.mu_drhoT(rho_nodal, Tk_nodal, mu, dmu_drho, dmu_dT);
  _viscosity_nodal[_qp] = mu;
  _dviscosity_nodal_dp[_qp] = dmu_drho * drho_dp_nodal;
  _dviscosity_nodal_dT[_qp] = dmu_dT;

  // Internal energy and derivatives wrt pressure and temperature at the nodes
  Real e_nodal, de_dp_nodal, de_dT_nodal;
  _fp.e_dpT(_porepressure_nodal[_qp][_phase_num], Tk_nodal, e_nodal, de_dp_nodal, de_dT_nodal);
  _internal_energy_nodal[_qp] = e_nodal;
  _dinternal_energy_nodal_dp[_qp] = de_dp_nodal;
  _dinternal_energy_nodal_dT[_qp] = de_dT_nodal;

  // Internal energy and derivatives wrt pressure and temperature at the qps
  Real e_qp, de_dp_qp, de_dT_qp;
  _fp.e_dpT(_porepressure_qp[_qp][_phase_num], Tk_qp, e_qp, de_dp_qp, de_dT_qp);
  _internal_energy_qp[_qp] = e_qp;
  _dinternal_energy_qp_dp[_qp] = de_dp_qp;
  _dinternal_energy_qp_dT[_qp] = de_dT_qp;

  // Enthalpy and derivatives wrt pressure and temperature at the nodes
  Real h_nodal, dh_dp_nodal, dh_dT_nodal;
  _fp.h_dpT(_porepressure_nodal[_qp][_phase_num], Tk_nodal, h_nodal, dh_dp_nodal, dh_dT_nodal);
  _enthalpy_nodal[_qp] = h_nodal;
  _denthalpy_nodal_dp[_qp] = dh_dp_nodal;
  _denthalpy_nodal_dT[_qp] = dh_dT_nodal;

  // Enthalpy and derivatives wrt pressure and temperature at the qps
  Real h_qp, dh_dp_qp, dh_dT_qp;
  _fp.h_dpT(_porepressure_qp[_qp][_phase_num], Tk_qp, h_qp, dh_dp_qp, dh_dT_qp);
  _enthalpy_qp[_qp] = h_qp;
  _denthalpy_qp_dp[_qp] = dh_dp_qp;
  _denthalpy_qp_dT[_qp] = dh_dT_qp;
}
