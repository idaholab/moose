/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowIdealGas.h"

template<>
InputParameters validParams<PorousFlowIdealGas>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addRequiredParam<Real>("molar_mass", "The molar mass of the Ideal gas (kg/mol)");
  params.addClassDescription("This Material calculates fluid density for an ideal gas");
  return params;
}

PorousFlowIdealGas::PorousFlowIdealGas(const InputParameters & parameters) :
    PorousFlowFluidPropertiesBase(parameters),

    _molar_mass(getParam<Real>("molar_mass")),
    _density_nodal(declareProperty<Real>("PorousFlow_fluid_phase_density" + _phase)),
    _density_nodal_old(declarePropertyOld<Real>("PorousFlow_fluid_phase_density" + _phase)),
    _ddensity_nodal_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + _phase, _pressure_variable_name)),
    _ddensity_nodal_dt(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + _phase, _temperature_variable_name)),
    _density_qp(declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + _phase)),
    _ddensity_qp_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _pressure_variable_name)),
    _ddensity_qp_dt(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _temperature_variable_name))
{
}

void
PorousFlowIdealGas::initQpStatefulProperties()
{
  _density_nodal[_qp] = density(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp], _molar_mass);
}

void
PorousFlowIdealGas::computeQpProperties()
{
  /// Density and derivatives wrt pressure and temperature at the nodes
  _density_nodal[_qp] = density(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp], _molar_mass);
  _ddensity_nodal_dp[_qp] = dDensity_dP(_temperature_nodal[_qp], _molar_mass);
  _ddensity_nodal_dt[_qp] = dDensity_dT(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp], _molar_mass);

  /// Density and derivatives wrt pressure and temperature at the qps
  _density_qp[_qp] = density(_porepressure_qp[_qp][_phase_num], _temperature_qp[_qp], _molar_mass);
  _ddensity_qp_dp[_qp] = dDensity_dP(_temperature_qp[_qp], _molar_mass);
  _ddensity_qp_dt[_qp] = dDensity_dT(_porepressure_qp[_qp][_phase_num], _temperature_qp[_qp], _molar_mass);
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
  return - pressure * molar_mass / (_R * tk * tk);
}
