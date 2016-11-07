/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowEnthalpy.h"
#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowEnthalpy>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addParam<Real>("porepressure_coefficient", 1.0, "The enthalpy is internal_energy + P / density * porepressure_contribution.  Physically this should be 1.0, but analytic solutions are simplified when it is zero");
  params.addClassDescription("This Material calculates fluid specific enthalpy (J/kg)");
  return params;
}

PorousFlowEnthalpy::PorousFlowEnthalpy(const InputParameters & parameters) :
    PorousFlowFluidPropertiesBase(parameters),
    _pp_coeff(getParam<Real>("porepressure_coefficient")),

    _energy_nodal(getMaterialProperty<Real>("PorousFlow_fluid_phase_internal_energy_nodal" + Moose::stringify(_phase_num))),
    _denergy_nodal_dp(getMaterialPropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_nodal" + Moose::stringify(_phase_num), _pressure_variable_name)),
    _denergy_nodal_dt(getMaterialPropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_nodal" + Moose::stringify(_phase_num), _temperature_variable_name)),
    _energy_qp(getMaterialProperty<Real>("PorousFlow_fluid_phase_internal_energy_qp" + Moose::stringify(_phase_num))),
    _denergy_qp_dp(getMaterialPropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_qp" + Moose::stringify(_phase_num), _pressure_variable_name)),
    _denergy_qp_dt(getMaterialPropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_qp" + Moose::stringify(_phase_num), _temperature_variable_name)),

    _density_nodal(getMaterialProperty<Real>("PorousFlow_fluid_phase_density" + Moose::stringify(_phase_num))),
    _ddensity_nodal_dp(getMaterialPropertyDerivative<Real>("PorousFlow_fluid_phase_density" + Moose::stringify(_phase_num), _pressure_variable_name)),
    _ddensity_nodal_dt(getMaterialPropertyDerivative<Real>("PorousFlow_fluid_phase_density" + Moose::stringify(_phase_num), _temperature_variable_name)),
    _density_qp(getMaterialProperty<Real>("PorousFlow_fluid_phase_density_qp" + Moose::stringify(_phase_num))),
    _ddensity_qp_dp(getMaterialPropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + Moose::stringify(_phase_num), _pressure_variable_name)),
    _ddensity_qp_dt(getMaterialPropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + Moose::stringify(_phase_num), _temperature_variable_name)),

    _enthalpy_nodal(declareProperty<Real>("PorousFlow_fluid_phase_enthalpy_nodal" + _phase)),
    _enthalpy_nodal_old(declarePropertyOld<Real>("PorousFlow_fluid_phase_enthalpy_nodal" + _phase)),
    _denthalpy_nodal_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_nodal" + _phase, _pressure_variable_name)),
    _denthalpy_nodal_dt(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_nodal" + _phase, _temperature_variable_name)),
    _enthalpy_qp(declareProperty<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase)),
    _denthalpy_qp_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase, _pressure_variable_name)),
    _denthalpy_qp_dt(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase, _temperature_variable_name))
{
}

void
PorousFlowEnthalpy::initQpStatefulProperties()
{
  _enthalpy_nodal[_qp] = _energy_nodal[_qp] + _porepressure_nodal[_qp][_phase_num] / _density_nodal[_qp] * _pp_coeff;
}

void
PorousFlowEnthalpy::computeQpProperties()
{
  /// Enthalpy and derivatives wrt pressure and temperature at the nodes
  _enthalpy_nodal[_qp] = _energy_nodal[_qp] + _porepressure_nodal[_qp][_phase_num] / _density_nodal[_qp] * _pp_coeff;
  _denthalpy_nodal_dp[_qp] = _denergy_nodal_dp[_qp] + (1.0 / _density_nodal[_qp] - _porepressure_nodal[_qp][_phase_num] * _ddensity_nodal_dp[_qp] / std::pow(_density_nodal[_qp], 2)) * _pp_coeff;
  _denthalpy_nodal_dt[_qp] = _denergy_nodal_dt[_qp] - _porepressure_nodal[_qp][_phase_num] * _ddensity_nodal_dt[_qp] / std::pow(_density_nodal[_qp], 2) * _pp_coeff;

  /// Enthalpy and derivatives wrt pressure and temperature at the qps
  _enthalpy_qp[_qp] = _energy_qp[_qp] + _porepressure_qp[_qp][_phase_num] / _density_qp[_qp] * _pp_coeff;
  _denthalpy_qp_dp[_qp] = _denergy_qp_dp[_qp] + (1.0 / _density_qp[_qp] - _porepressure_qp[_qp][_phase_num] * _ddensity_qp_dp[_qp] / std::pow(_density_qp[_qp], 2)) * _pp_coeff;
  _denthalpy_qp_dt[_qp] = _denergy_qp_dt[_qp] - _porepressure_qp[_qp][_phase_num] * _ddensity_qp_dt[_qp] / std::pow(_density_qp[_qp], 2) * _pp_coeff;
}
