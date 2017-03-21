/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowIdealGas.h"

template <>
InputParameters
validParams<PorousFlowIdealGas>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addRequiredParam<Real>("molar_mass", "The molar mass of the Ideal gas (kg/mol)");
  params.addClassDescription("This Material calculates fluid density for an ideal gas");
  return params;
}

PorousFlowIdealGas::PorousFlowIdealGas(const InputParameters & parameters)
  : PorousFlowFluidPropertiesBase(parameters),

    _molar_mass(getParam<Real>("molar_mass")),
    _density(_nodal_material
                 ? declareProperty<Real>("PorousFlow_fluid_phase_density_nodal" + _phase)
                 : declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + _phase)),
    _ddensity_dp(_nodal_material
                     ? declarePropertyDerivative<Real>(
                           "PorousFlow_fluid_phase_density_nodal" + _phase, _pressure_variable_name)
                     : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase,
                                                       _pressure_variable_name)),
    _ddensity_dt(
        _nodal_material
            ? declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_nodal" + _phase,
                                              _temperature_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase,
                                              _temperature_variable_name))
{
}

void
PorousFlowIdealGas::initQpStatefulProperties()
{
  _density[_qp] = density(_porepressure[_qp][_phase_num], _temperature[_qp], _molar_mass);
}

void
PorousFlowIdealGas::computeQpProperties()
{
  _density[_qp] = density(_porepressure[_qp][_phase_num], _temperature[_qp], _molar_mass);
  _ddensity_dp[_qp] = dDensity_dP(_temperature[_qp], _molar_mass);
  _ddensity_dt[_qp] = dDensity_dT(_porepressure[_qp][_phase_num], _temperature[_qp], _molar_mass);
}

Real
PorousFlowIdealGas::density(Real pressure, Real temperature, Real molar_mass) const
{
  return pressure * molar_mass / (_R * (temperature + _t_c2k));
}

Real
PorousFlowIdealGas::dDensity_dP(Real temperature, Real molar_mass) const
{
  return molar_mass / (_R * (temperature + _t_c2k));
}

Real
PorousFlowIdealGas::dDensity_dT(Real pressure, Real temperature, Real molar_mass) const
{
  Real tk = temperature + _t_c2k;
  return -pressure * molar_mass / (_R * tk * tk);
}
