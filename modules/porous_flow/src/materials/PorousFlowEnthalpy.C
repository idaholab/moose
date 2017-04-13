/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowEnthalpy.h"

template <>
InputParameters
validParams<PorousFlowEnthalpy>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addParam<Real>("porepressure_coefficient",
                        1.0,
                        "The enthalpy is internal_energy + P / density * "
                        "porepressure_coefficient.  Physically this should be 1.0, "
                        "but analytic solutions are simplified when it is zero");
  params.addClassDescription(
      "This Material calculates fluid specific enthalpy (J/kg) at the nodes or quadpoints");
  return params;
}

PorousFlowEnthalpy::PorousFlowEnthalpy(const InputParameters & parameters)
  : PorousFlowFluidPropertiesBase(parameters),
    _pp_coeff(getParam<Real>("porepressure_coefficient")),

    _energy(_nodal_material
                ? getMaterialProperty<Real>("PorousFlow_fluid_phase_internal_energy_nodal" + _phase)
                : getMaterialProperty<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase)),
    _denergy_dp(
        _nodal_material
            ? getMaterialPropertyDerivative<Real>(
                  "PorousFlow_fluid_phase_internal_energy_nodal" + _phase, _pressure_variable_name)
            : getMaterialPropertyDerivative<Real>(
                  "PorousFlow_fluid_phase_internal_energy_qp" + _phase, _pressure_variable_name)),
    _denergy_dt(_nodal_material ? getMaterialPropertyDerivative<Real>(
                                      "PorousFlow_fluid_phase_internal_energy_nodal" + _phase,
                                      _temperature_variable_name)
                                : getMaterialPropertyDerivative<Real>(
                                      "PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                      _temperature_variable_name)),

    _density(_nodal_material
                 ? getMaterialProperty<Real>("PorousFlow_fluid_phase_density_nodal" + _phase)
                 : getMaterialProperty<Real>("PorousFlow_fluid_phase_density_qp" + _phase)),
    _ddensity_dp(_nodal_material
                     ? getMaterialPropertyDerivative<Real>(
                           "PorousFlow_fluid_phase_density_nodal" + _phase, _pressure_variable_name)
                     : getMaterialPropertyDerivative<Real>(
                           "PorousFlow_fluid_phase_density_qp" + _phase, _pressure_variable_name)),
    _ddensity_dt(
        _nodal_material
            ? getMaterialPropertyDerivative<Real>("PorousFlow_fluid_phase_density_nodal" + _phase,
                                                  _temperature_variable_name)
            : getMaterialPropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase,
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
    _denthalpy_dt(
        _nodal_material
            ? declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_nodal" + _phase,
                                              _temperature_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_enthalpy_qp" + _phase,
                                              _temperature_variable_name))
{
}

void
PorousFlowEnthalpy::initQpStatefulProperties()
{
  _enthalpy[_qp] = _energy[_qp] + _porepressure[_qp][_phase_num] / _density[_qp] * _pp_coeff;
}

void
PorousFlowEnthalpy::computeQpProperties()
{
  _enthalpy[_qp] = _energy[_qp] + _porepressure[_qp][_phase_num] / _density[_qp] * _pp_coeff;
  _denthalpy_dp[_qp] =
      _denergy_dp[_qp] +
      (1.0 / _density[_qp] -
       _porepressure[_qp][_phase_num] * _ddensity_dp[_qp] / std::pow(_density[_qp], 2)) *
          _pp_coeff;
  _denthalpy_dt[_qp] =
      _denergy_dt[_qp] -
      _porepressure[_qp][_phase_num] * _ddensity_dt[_qp] / std::pow(_density[_qp], 2) * _pp_coeff;
}
