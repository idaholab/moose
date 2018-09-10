//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowSingleComponentFluid.h"

registerMooseObject("PorousFlowApp", PorousFlowSingleComponentFluid);

template <>
InputParameters
validParams<PorousFlowSingleComponentFluid>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addParam<bool>(
      "compute_density_and_viscosity", true, "Compute the fluid density and viscosity");
  params.addParam<bool>("compute_internal_energy", true, "Compute the fluid internal energy");
  params.addParam<bool>("compute_enthalpy", true, "Compute the fluid enthalpy");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription(
      "This Material calculates fluid properties at the quadpoints for a single component fluid");
  return params;
}

PorousFlowSingleComponentFluid::PorousFlowSingleComponentFluid(const InputParameters & parameters)
  : PorousFlowFluidPropertiesBase(parameters),
    _compute_rho_mu(getParam<bool>("compute_density_and_viscosity")),
    _compute_internal_energy(getParam<bool>("compute_internal_energy")),
    _compute_enthalpy(getParam<bool>("compute_enthalpy")),
    _density(_compute_rho_mu
                 ? (_nodal_material
                        ? &declareProperty<Real>("PorousFlow_fluid_phase_density_nodal" + _phase)
                        : &declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + _phase))
                 : nullptr),
    _ddensity_dp(
        _compute_rho_mu
            ? (_nodal_material
                   ? &declarePropertyDerivative<Real>(
                         "PorousFlow_fluid_phase_density_nodal" + _phase, _pressure_variable_name)
                   : &declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase,
                                                      _pressure_variable_name))
            : nullptr),
    _ddensity_dT(_compute_rho_mu
                     ? (_nodal_material ? &declarePropertyDerivative<Real>(
                                              "PorousFlow_fluid_phase_density_nodal" + _phase,
                                              _temperature_variable_name)
                                        : &declarePropertyDerivative<Real>(
                                              "PorousFlow_fluid_phase_density_qp" + _phase,
                                              _temperature_variable_name))
                     : nullptr),

    _viscosity(_compute_rho_mu
                   ? (_nodal_material
                          ? &declareProperty<Real>("PorousFlow_viscosity_nodal" + _phase)
                          : &declareProperty<Real>("PorousFlow_viscosity_qp" + _phase))
                   : nullptr),
    _dviscosity_dp(_compute_rho_mu
                       ? (_nodal_material
                              ? &declarePropertyDerivative<Real>(
                                    "PorousFlow_viscosity_nodal" + _phase, _pressure_variable_name)
                              : &declarePropertyDerivative<Real>("PorousFlow_viscosity_qp" + _phase,
                                                                 _pressure_variable_name))
                       : nullptr),
    _dviscosity_dT(
        _compute_rho_mu
            ? (_nodal_material
                   ? &declarePropertyDerivative<Real>("PorousFlow_viscosity_nodal" + _phase,
                                                      _temperature_variable_name)
                   : &declarePropertyDerivative<Real>("PorousFlow_viscosity_qp" + _phase,
                                                      _temperature_variable_name))
            : nullptr),

    _internal_energy(
        _compute_internal_energy
            ? (_nodal_material
                   ? &declareProperty<Real>("PorousFlow_fluid_phase_internal_energy_nodal" + _phase)
                   : &declareProperty<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase))
            : nullptr),
    _dinternal_energy_dp(_compute_internal_energy
                             ? (_nodal_material
                                    ? &declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_nodal" + _phase,
                                          _pressure_variable_name)
                                    : &declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                          _pressure_variable_name))
                             : nullptr),
    _dinternal_energy_dT(_compute_internal_energy
                             ? (_nodal_material
                                    ? &declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_nodal" + _phase,
                                          _temperature_variable_name)
                                    : &declarePropertyDerivative<Real>(
                                          "PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                          _temperature_variable_name))
                             : nullptr),

    _enthalpy(_compute_enthalpy
                  ? (_nodal_material
                         ? &declareProperty<Real>("PorousFlow_fluid_phase_enthalpy_nodal" + _phase)
                         : &declareProperty<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase))
                  : nullptr),
    _denthalpy_dp(
        _compute_enthalpy
            ? (_nodal_material
                   ? &declarePropertyDerivative<Real>(
                         "PorousFlow_fluid_phase_enthalpy_nodal" + _phase, _pressure_variable_name)
                   : &declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase,
                                                      _pressure_variable_name))
            : nullptr),
    _denthalpy_dT(_compute_enthalpy
                      ? (_nodal_material ? &declarePropertyDerivative<Real>(
                                               "PorousFlow_fluid_phase_enthalpy_nodal" + _phase,
                                               _temperature_variable_name)
                                         : &declarePropertyDerivative<Real>(
                                               "PorousFlow_fluid_phase_enthalpy_qp" + _phase,
                                               _temperature_variable_name))
                      : nullptr),

    _fp(getUserObject<SinglePhaseFluidPropertiesPT>("fp"))
{
}

void
PorousFlowSingleComponentFluid::initQpStatefulProperties()
{
  if (_compute_rho_mu)
    (*_density)[_qp] = _fp.rho_from_p_T(_porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k);
  if (_compute_internal_energy)
    (*_internal_energy)[_qp] =
        _fp.e_from_p_T(_porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k);
  if (_compute_enthalpy)
    (*_enthalpy)[_qp] = _fp.h_from_p_T(_porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k);
}

void
PorousFlowSingleComponentFluid::computeQpProperties()
{
  const Real Tk = _temperature[_qp] + _t_c2k;

  if (_compute_rho_mu)
  {
    // Density and viscosity, and derivatives wrt pressure and temperature
    Real rho, drho_dp, drho_dT, mu, dmu_dp, dmu_dT;
    _fp.rho_mu_dpT(_porepressure[_qp][_phase_num], Tk, rho, drho_dp, drho_dT, mu, dmu_dp, dmu_dT);
    (*_density)[_qp] = rho;
    (*_ddensity_dp)[_qp] = drho_dp;
    (*_ddensity_dT)[_qp] = drho_dT;
    (*_viscosity)[_qp] = mu;
    (*_dviscosity_dp)[_qp] = dmu_dp;
    (*_dviscosity_dT)[_qp] = dmu_dT;
  }

  // Internal energy and derivatives wrt pressure and temperature at the qps
  if (_compute_internal_energy)
  {
    Real e, de_dp, de_dT;
    _fp.e_from_p_T(_porepressure[_qp][_phase_num], Tk, e, de_dp, de_dT);
    (*_internal_energy)[_qp] = e;
    (*_dinternal_energy_dp)[_qp] = de_dp;
    (*_dinternal_energy_dT)[_qp] = de_dT;
  }

  // Enthalpy and derivatives wrt pressure and temperature at the qps
  if (_compute_enthalpy)
  {
    Real h, dh_dp, dh_dT;
    _fp.h_from_p_T(_porepressure[_qp][_phase_num], Tk, h, dh_dp, dh_dT);
    (*_enthalpy)[_qp] = h;
    (*_denthalpy_dp)[_qp] = dh_dp;
    (*_denthalpy_dT)[_qp] = dh_dT;
  }
}
