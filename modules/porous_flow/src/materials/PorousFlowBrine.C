/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowBrine.h"

template <>
InputParameters
validParams<PorousFlowBrine>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addCoupledVar("xnacl", 0, "The salt mass fraction in the brine (kg/kg)");
  params.addClassDescription(
      "This Material calculates fluid properties for brine at the quadpoints or nodes");
  return params;
}

PorousFlowBrine::PorousFlowBrine(const InputParameters & parameters)
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
            ? declareProperty<Real>("PorousFlow_fluid_phase_internal_energy" + _phase)
            : declareProperty<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase)),
    _dinternal_energy_dp(
        _nodal_material
            ? declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy" + _phase,
                                              _pressure_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                              _pressure_variable_name)),
    _dinternal_energy_dT(
        _nodal_material
            ? declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy" + _phase,
                                              _temperature_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                              _temperature_variable_name)),

    _enthalpy(_nodal_material
                  ? declareProperty<Real>("PorousFlow_fluid_phase_enthalpy" + _phase)
                  : declareProperty<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase)),
    _denthalpy_dp(
        _nodal_material
            ? declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy" + _phase,
                                              _pressure_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase,
                                              _pressure_variable_name)),
    _denthalpy_dT(
        _nodal_material
            ? declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy" + _phase,
                                              _temperature_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase,
                                              _temperature_variable_name)),

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
  _density[_qp] =
      _brine_fp->rho(_porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k, _xnacl[_qp]);
  _internal_energy[_qp] =
      _brine_fp->e(_porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k, _xnacl[_qp]);
  _enthalpy[_qp] =
      _brine_fp->h(_porepressure[_qp][_phase_num], _temperature[_qp] + _t_c2k, _xnacl[_qp]);
}

void
PorousFlowBrine::computeQpProperties()
{
  // Density and derivatives wrt pressure and temperature at the nodes
  Real Tk = _temperature[_qp] + _t_c2k;
  Real rho, drho_dp, drho_dT, drho_dx;
  _brine_fp->rho_dpTx(
      _porepressure[_qp][_phase_num], Tk, _xnacl[_qp], rho, drho_dp, drho_dT, drho_dx);
  _density[_qp] = rho;
  _ddensity_dp[_qp] = drho_dp;
  _ddensity_dT[_qp] = drho_dT;

  // Viscosity and derivatives wrt pressure and temperature at the nodes.
  // Note that dmu_dp = dmu_drho * drho_dp
  Real mu, dmu_drho, dmu_dT, dmu_dx;
  // Viscosity calculation requires water density
  Real rhow, drhow_dp, drhow_dT;
  _water_fp->rho_dpT(_porepressure[_qp][_phase_num], Tk, rhow, drhow_dp, drhow_dT);
  _brine_fp->mu_drhoTx(rhow, Tk, _xnacl[_qp], mu, dmu_drho, dmu_dT, dmu_dx);
  _viscosity[_qp] = mu;
  _dviscosity_dp[_qp] = dmu_drho * drhow_dp;
  _dviscosity_dT[_qp] = dmu_dT;

  // Internal energy and derivatives wrt pressure and temperature at the nodes
  Real e, de_dp, de_dT, de_dx;
  _brine_fp->e_dpTx(_porepressure[_qp][_phase_num], Tk, _xnacl[_qp], e, de_dp, de_dT, de_dx);
  _internal_energy[_qp] = e;
  _dinternal_energy_dp[_qp] = de_dp;
  _dinternal_energy_dT[_qp] = de_dT;

  // Enthalpy and derivatives wrt pressure and temperature at the nodes
  Real h, dh_dp, dh_dT, dh_dx;
  _brine_fp->h_dpTx(_porepressure[_qp][_phase_num], Tk, _xnacl[_qp], h, dh_dp, dh_dT, dh_dx);
  _enthalpy[_qp] = h;
  _denthalpy_dp[_qp] = dh_dp;
  _denthalpy_dT[_qp] = dh_dT;
}
