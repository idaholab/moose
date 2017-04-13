/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowSingleComponentFluid.h"

template <>
InputParameters
validParams<PorousFlowSingleComponentFluid>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription(
      "This Material calculates fluid properties at the quadpoints for a single component fluid");
  return params;
}

PorousFlowSingleComponentFluid::PorousFlowSingleComponentFluid(const InputParameters & parameters)
  : PorousFlowFluidPropertiesBase(parameters),

    _density(_nodal_material
                 ? declareProperty<Real>("PorousFlow_fluid_phase_density_nodal" + _phase)
                 : declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + _phase)),
    _ddensity_dp(_nodal_material
                     ? declarePropertyDerivative<Real>(
                           "PorousFlow_fluid_phase_density_nodal" + _phase, _pressure_variable_name)
                     : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase,
                                                       _pressure_variable_name)),
    _ddensity_dT(
        _nodal_material
            ? declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_nodal" + _phase,
                                              _temperature_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase,
                                              _temperature_variable_name)),

    _viscosity(_nodal_material ? declareProperty<Real>("PorousFlow_viscosity_nodal" + _phase)
                               : declareProperty<Real>("PorousFlow_viscosity_qp" + _phase)),
    _dviscosity_dp(_nodal_material
                       ? declarePropertyDerivative<Real>("PorousFlow_viscosity_nodal" + _phase,
                                                         _pressure_variable_name)
                       : declarePropertyDerivative<Real>("PorousFlow_viscosity_qp" + _phase,
                                                         _pressure_variable_name)),
    _dviscosity_dT(_nodal_material
                       ? declarePropertyDerivative<Real>("PorousFlow_viscosity_nodal" + _phase,
                                                         _temperature_variable_name)
                       : declarePropertyDerivative<Real>("PorousFlow_viscosity_qp" + _phase,
                                                         _temperature_variable_name)),

    _internal_energy(
        _nodal_material
            ? declareProperty<Real>("PorousFlow_fluid_phase_internal_energy_nodal" + _phase)
            : declareProperty<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase)),
    _dinternal_energy_dp(
        _nodal_material
            ? declarePropertyDerivative<Real>(
                  "PorousFlow_fluid_phase_internal_energy_nodal" + _phase, _pressure_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                              _pressure_variable_name)),
    _dinternal_energy_dT(
        _nodal_material
            ? declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_nodal" +
                                                  _phase,
                                              _temperature_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                              _temperature_variable_name)),

    _enthalpy(_nodal_material
                  ? declareProperty<Real>("PorousFlow_fluid_phase_enthalpy_nodal" + _phase)
                  : declareProperty<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase)),
    _denthalpy_dp(
        _nodal_material
            ? declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_nodal" + _phase,
                                              _pressure_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase,
                                              _pressure_variable_name)),
    _denthalpy_dT(
        _nodal_material
            ? declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_nodal" + _phase,
                                              _temperature_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase,
                                              _temperature_variable_name)),

    _fp(getUserObject<SinglePhaseFluidPropertiesPT>("fp"))
{
}

void
PorousFlowSingleComponentFluid::initQpStatefulProperties()
{
  _density[_qp] = _fp.rho(_porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k);
  _internal_energy[_qp] = _fp.e(_porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k);
  _enthalpy[_qp] = _fp.h(_porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k);
}

void
PorousFlowSingleComponentFluid::computeQpProperties()
{
  // Density and derivatives wrt pressure and temperature at the qps
  const Real Tk = _temperature[_qp] + _t_c2k;
  Real rho, drho_dp, drho_dT;
  _fp.rho_dpT(_porepressure[_qp][_phase_num], Tk, rho, drho_dp, drho_dT);
  _density[_qp] = rho;
  _ddensity_dp[_qp] = drho_dp;
  _ddensity_dT[_qp] = drho_dT;

  // Viscosity and derivatives wrt pressure and temperature at the nodes.
  // Note that dmu_dp = dmu_drho * drho_dp
  Real mu, dmu_drho, dmu_dT;
  _fp.mu_drhoT(rho, Tk, mu, dmu_drho, dmu_dT);
  _viscosity[_qp] = mu;
  _dviscosity_dp[_qp] = dmu_drho * drho_dp;
  _dviscosity_dT[_qp] = dmu_dT;

  // Internal energy and derivatives wrt pressure and temperature at the qps
  Real e, de_dp, de_dT;
  _fp.e_dpT(_porepressure[_qp][_phase_num], Tk, e, de_dp, de_dT);
  _internal_energy[_qp] = e;
  _dinternal_energy_dp[_qp] = de_dp;
  _dinternal_energy_dT[_qp] = de_dT;

  // Enthalpy and derivatives wrt pressure and temperature at the qps
  Real h, dh_dp, dh_dT;
  _fp.h_dpT(_porepressure[_qp][_phase_num], Tk, h, dh_dp, dh_dT);
  _enthalpy[_qp] = h;
  _denthalpy_dp[_qp] = dh_dp;
  _denthalpy_dT[_qp] = dh_dT;
}
