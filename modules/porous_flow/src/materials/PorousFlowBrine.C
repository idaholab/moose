/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowBrine.h"

template<>
InputParameters validParams<PorousFlowBrine>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addCoupledVar("xnacl", 0, "The salt mass fraction in the brine (kg/kg)");
  params.addClassDescription("This Material calculates fluid properties for brine at the quadpoints");
  return params;
}

PorousFlowBrine::PorousFlowBrine(const InputParameters & parameters) :
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

    _xnacl_qp(coupledValue("xnacl"))
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

  _nodal_material = false;
}

void
PorousFlowBrine::computeQpProperties()
{
  // Density and derivatives wrt pressure and temperature at the qps
  Real Tk_qp = _temperature_qp[_qp] + _t_c2k;
  Real rho_qp, drho_dp_qp, drho_dT_qp, drho_dx_qp;
  _brine_fp->rho_dpTx(_porepressure_qp[_qp][_phase_num], Tk_qp, _xnacl_qp[_qp], rho_qp, drho_dp_qp, drho_dT_qp, drho_dx_qp);
  _density_qp[_qp] = rho_qp;
  _ddensity_qp_dp[_qp] = drho_dp_qp;
  _ddensity_qp_dT[_qp] = drho_dT_qp;

  // Internal energy and derivatives wrt pressure and temperature at the qps
  Real e_qp, de_dp_qp, de_dT_qp, de_dx_qp;
  _brine_fp->e_dpTx(_porepressure_qp[_qp][_phase_num], Tk_qp, _xnacl_qp[_qp], e_qp, de_dp_qp, de_dT_qp, de_dx_qp);
  _internal_energy_qp[_qp] = e_qp;
  _dinternal_energy_qp_dp[_qp] = de_dp_qp;
  _dinternal_energy_qp_dT[_qp] = de_dT_qp;

  // Enthalpy and derivatives wrt pressure and temperature at the qps
  Real h_qp, dh_dp_qp, dh_dT_qp, dh_dx_qp;
  _brine_fp->h_dpTx(_porepressure_qp[_qp][_phase_num], Tk_qp, _xnacl_qp[_qp], h_qp, dh_dp_qp, dh_dT_qp, dh_dx_qp);
  _enthalpy_qp[_qp] = h_qp;
  _denthalpy_qp_dp[_qp] = dh_dp_qp;
  _denthalpy_qp_dT[_qp] = dh_dT_qp;
}
