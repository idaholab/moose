//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowBrine.h"

template <>
InputParameters
validParams<PorousFlowBrine>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addParam<bool>(
      "compute_density_and_viscosity", true, "Compute the fluid density and viscosity");
  params.addParam<bool>("compute_internal_energy", true, "Compute the fluid internal energy");
  params.addParam<bool>("compute_enthalpy", true, "Compute the fluid enthalpy");
  params.addCoupledVar("xnacl", 0, "The salt mass fraction in the brine (kg/kg)");
  params.addClassDescription(
      "This Material calculates fluid properties for brine at the quadpoints or nodes");
  return params;
}

PorousFlowBrine::PorousFlowBrine(const InputParameters & parameters)
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
    _ddensity_dT(
        _compute_rho_mu
            ? (_nodal_material
                   ? &declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_nodal" +
                                                          _phase,
                                                      _temperature_variable_name)
                   : &declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase,
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
    _denthalpy_dT(
        _compute_enthalpy
            ? (_nodal_material
                   ? &declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_nodal" +
                                                          _phase,
                                                      _temperature_variable_name)
                   : &declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase,
                                                      _temperature_variable_name))
            : nullptr),

    _xnacl(_nodal_material ? coupledNodalValue("xnacl") : coupledValue("xnacl"))
{
  // BrineFluidProperties UserObject
  std::string brine_name = name() + ":brine";
  {
    std::string class_name = "BrineFluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);
    _fe_problem.addUserObject(class_name, brine_name, params);
  }
  _brine_fp = &_fe_problem.getUserObject<BrineFluidProperties>(brine_name);

  // Water properties UserObject
  _water_fp = &_brine_fp->getComponent(BrineFluidProperties::WATER);
}

void
PorousFlowBrine::initQpStatefulProperties()
{
  if (_compute_rho_mu)
    (*_density)[_qp] =
        _brine_fp->rho(_porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k, _xnacl[_qp]);
  if (_compute_internal_energy)
    (*_internal_energy)[_qp] =
        _brine_fp->e(_porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k, _xnacl[_qp]);
  if (_compute_enthalpy)
    (*_enthalpy)[_qp] =
        _brine_fp->h(_porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k, _xnacl[_qp]);
}

void
PorousFlowBrine::computeQpProperties()
{
  const Real Tk = _temperature[_qp] + _t_c2k;

  if (_compute_rho_mu)
  {
    // Density and derivatives wrt pressure and temperature at the nodes
    Real rho, drho_dp, drho_dT, drho_dx;
    _brine_fp->rho_dpTx(
        _porepressure[_qp][_phase_num], Tk, _xnacl[_qp], rho, drho_dp, drho_dT, drho_dx);
    (*_density)[_qp] = rho;
    (*_ddensity_dp)[_qp] = drho_dp;
    (*_ddensity_dT)[_qp] = drho_dT;

    // Viscosity and derivatives wrt pressure and temperature at the nodes.
    // Note that dmu_dp = dmu_drho * drho_dp
    Real mu, dmu_drho, dmu_dT, dmu_dx;
    // Viscosity calculation requires water density
    Real rhow, drhow_dp, drhow_dT;
    _water_fp->rho_dpT(_porepressure[_qp][_phase_num], Tk, rhow, drhow_dp, drhow_dT);
    _brine_fp->mu_drhoTx(rhow, Tk, _xnacl[_qp], drhow_dT, mu, dmu_drho, dmu_dT, dmu_dx);
    (*_viscosity)[_qp] = mu;
    (*_dviscosity_dp)[_qp] = dmu_drho * drhow_dp;
    (*_dviscosity_dT)[_qp] = dmu_dT;
  }

  // Internal energy and derivatives wrt pressure and temperature at the nodes
  if (_compute_internal_energy)
  {
    Real e, de_dp, de_dT, de_dx;
    _brine_fp->e_dpTx(_porepressure[_qp][_phase_num], Tk, _xnacl[_qp], e, de_dp, de_dT, de_dx);
    (*_internal_energy)[_qp] = e;
    (*_dinternal_energy_dp)[_qp] = de_dp;
    (*_dinternal_energy_dT)[_qp] = de_dT;
  }

  // Enthalpy and derivatives wrt pressure and temperature at the nodes
  if (_compute_enthalpy)
  {
    Real h, dh_dp, dh_dT, dh_dx;
    _brine_fp->h_dpTx(_porepressure[_qp][_phase_num], Tk, _xnacl[_qp], h, dh_dp, dh_dT, dh_dx);
    (*_enthalpy)[_qp] = h;
    (*_denthalpy_dp)[_qp] = dh_dp;
    (*_denthalpy_dT)[_qp] = dh_dT;
  }
}
