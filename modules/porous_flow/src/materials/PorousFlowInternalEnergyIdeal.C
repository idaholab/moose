/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowInternalEnergyIdeal.h"

template <>
InputParameters
validParams<PorousFlowInternalEnergyIdeal>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addRequiredParam<Real>(
      "specific_heat_capacity",
      "The specific heat capactiy at constant volume of the ideal fluid (J/kg/K)");
  params.addClassDescription("This Material calculates fluid internal energy at the quadpoints or "
                             "nodes assuming a contant specific heat capacity");
  return params;
}

PorousFlowInternalEnergyIdeal::PorousFlowInternalEnergyIdeal(const InputParameters & parameters)
  : PorousFlowFluidPropertiesBase(parameters),

    _cv(getParam<Real>("specific_heat_capacity")),
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
    _dinternal_energy_dt(
        _nodal_material
            ? declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_nodal" +
                                                  _phase,
                                              _temperature_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_internal_energy_qp" + _phase,
                                              _temperature_variable_name))
{
}

void
PorousFlowInternalEnergyIdeal::initQpStatefulProperties()
{
  _internal_energy[_qp] = _cv * _temperature[_qp];
}

void
PorousFlowInternalEnergyIdeal::computeQpProperties()
{
  _internal_energy[_qp] = _cv * _temperature[_qp];
  _dinternal_energy_dp[_qp] = 0.0;
  _dinternal_energy_dt[_qp] = _cv;
}
