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
  params.addParam<bool>("at_nodes", false, "Material properties will be calculated at nodes rather than the usual quadpoints");
  params.addRequiredParam<Real>("molar_mass", "The molar mass of the Ideal gas (kg/mol)");
  params.addClassDescription("This Material calculates fluid density for an ideal gas");
  return params;
}

PorousFlowIdealGas::PorousFlowIdealGas(const InputParameters & parameters) :
    PorousFlowFluidPropertiesBase(parameters),

    _molar_mass(getParam<Real>("molar_mass")),
    _density_nodal(getParam<bool>("at_nodes") ? &declareProperty<Real>("PorousFlow_fluid_phase_density" + _phase) : nullptr),
    _density_nodal_old(getParam<bool>("at_nodes") ? &declarePropertyOld<Real>("PorousFlow_fluid_phase_density" + _phase) : nullptr),
    _ddensity_nodal_dp(getParam<bool>("at_nodes") ? &declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + _phase, _pressure_variable_name) : nullptr),
    _ddensity_nodal_dt(getParam<bool>("at_nodes") ? &declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + _phase, _temperature_variable_name) : nullptr),

    _density_qp(!getParam<bool>("at_nodes") ? &declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + _phase) : nullptr),
    _ddensity_qp_dp(!getParam<bool>("at_nodes") ? &declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _pressure_variable_name) : nullptr),
    _ddensity_qp_dt(!getParam<bool>("at_nodes") ? &declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _temperature_variable_name) : nullptr)
{
  _nodal_material = getParam<bool>("at_nodes");
}

void
PorousFlowIdealGas::initQpStatefulProperties()
{
  if (_nodal_material)
    (*_density_nodal)[_qp] = density(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp], _molar_mass);
}

void
PorousFlowIdealGas::computeQpProperties()
{
  if (_nodal_material)
  {
    /// Density and derivatives wrt pressure and temperature at the nodes
    (*_density_nodal)[_qp] = density(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp], _molar_mass);
    (*_ddensity_nodal_dp)[_qp] = dDensity_dP(_temperature_nodal[_qp], _molar_mass);
    (*_ddensity_nodal_dt)[_qp] = dDensity_dT(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp], _molar_mass);
  }
  else
  {
    /// Density and derivatives wrt pressure and temperature at the qps
    (*_density_qp)[_qp] = density(_porepressure_qp[_qp][_phase_num], _temperature_qp[_qp], _molar_mass);
    (*_ddensity_qp_dp)[_qp] = dDensity_dP(_temperature_qp[_qp], _molar_mass);
    (*_ddensity_qp_dt)[_qp] = dDensity_dT(_porepressure_qp[_qp][_phase_num], _temperature_qp[_qp], _molar_mass);
  }
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
