/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMaterialMethane.h"
#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowMaterialMethane>()
{
  InputParameters params = validParams<PorousFlowMaterialFluidPropertiesBase>();
  params.addClassDescription("This Material calculates fluid properties for methane");
  return params;
}

PorousFlowMaterialMethane::PorousFlowMaterialMethane(const InputParameters & parameters) :
    PorousFlowMaterialFluidPropertiesBase(parameters),

  _density_nodal(declareProperty<Real>("PorousFlow_fluid_phase_density" + Moose::stringify(_phase_num))),
  _density_nodal_old(declarePropertyOld<Real>("PorousFlow_fluid_phase_density" + Moose::stringify(_phase_num))),
  _ddensity_nodal_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + Moose::stringify(_phase_num), _pressure_variable_name)),
  _ddensity_nodal_dt(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + Moose::stringify(_phase_num), _temperature_variable_name)),
  _density_qp(declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + Moose::stringify(_phase_num))),
  _ddensity_qp_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + Moose::stringify(_phase_num), _pressure_variable_name)),
  _ddensity_qp_dt(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + Moose::stringify(_phase_num), _temperature_variable_name)),
  _viscosity_nodal(declareProperty<Real>("PorousFlow_viscosity" + Moose::stringify(_phase_num))),
  _dviscosity_nodal_dt(declarePropertyDerivative<Real>("PorousFlow_viscosity" + Moose::stringify(_phase_num), _temperature_variable_name))
{
  _Mch4 = 16.0425e-3;
}

void
PorousFlowMaterialMethane::initQpStatefulProperties()
{
  _density_nodal[_qp] = density(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp][_phase_num]);
}

void
PorousFlowMaterialMethane::computeQpProperties()
{
  /// Density and derivatives wrt pressure and temperature at the nodes
  _density_nodal[_qp] = density(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp][_phase_num]);
  _ddensity_nodal_dp[_qp] = dDensity_dP(_temperature_nodal[_qp][_phase_num]);
  _ddensity_nodal_dt[_qp] = dDensity_dT(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp][_phase_num]);

  /// Density and derivatives wrt pressure and temperature at the qps
  _density_qp[_qp] = density(_porepressure_qp[_qp][_phase_num], _temperature_qp[_qp][_phase_num]);
  _ddensity_qp_dp[_qp] = dDensity_dP(_temperature_qp[_qp][_phase_num]);
  _ddensity_qp_dt[_qp] = dDensity_dT(_porepressure_qp[_qp][_phase_num], _temperature_qp[_qp][_phase_num]);

  /// Viscosity and derivative wrt temperature at the nodes
  _viscosity_nodal[_qp] = viscosity(_temperature_nodal[_qp][_phase_num]);
  _dviscosity_nodal_dt[_qp] = dViscosity_dT(_temperature_nodal[_qp][_phase_num]);
}

Real
PorousFlowMaterialMethane::density(Real pressure, Real temperature) const
{
  return pressure * _Mch4 / (_R * (temperature + _t_c2k));
}

Real
PorousFlowMaterialMethane::dDensity_dP(Real temperature) const
{
  return _Mch4 / (_R * (temperature + _t_c2k));
}

Real
PorousFlowMaterialMethane::dDensity_dT(Real pressure, Real temperature) const
{
  Real tk = temperature + _t_c2k;
  return - pressure * _Mch4 / (_R * tk * tk);
}

Real
PorousFlowMaterialMethane::viscosity(Real temperature) const
{
  Real a[6] = {2.968267e-1, 3.711201e-2, 1.218298e-5, -7.02426e-8, 7.543269e-11,
    -2.7237166e-14};

  Real viscosity;
  Real tk = temperature + _t_c2k;
  Real tk2 = tk * tk;
  Real tk3 = tk2 * tk;
  Real tk4 = tk3 * tk;
  Real tk5 = tk4 * tk;

  viscosity = a[0] + a[1] * tk + a[2] * tk2 + a[3] * tk3 + a[4] * tk4 + a[5] * tk5;

  return viscosity * 1.e-6;
}

Real
PorousFlowMaterialMethane::dViscosity_dT(Real temperature) const
{
  Real a[6] = {2.968267e-1, 3.711201e-2, 1.218298e-5, -7.02426e-8, 7.543269e-11,
    -2.7237166e-14};

  Real dviscosity;
  Real tk = temperature + _t_c2k;
  Real tk2 = tk * tk;
  Real tk3 = tk2 * tk;
  Real tk4 = tk3 * tk;

  dviscosity = a[1] + 2.0 * a[2] * tk + 3.0 * a[3] * tk2 + 4.0 * a[4] * tk3 + 5.0 * a[5] * tk4;

  return dviscosity * 1.e-6;
}

std::vector<Real>
PorousFlowMaterialMethane::henryConstants() const
{
  std::vector<Real> ch4henry;
  ch4henry.push_back(-10.44708);
  ch4henry.push_back(4.66491);
  ch4henry.push_back(12.12986);

  return ch4henry;
}
