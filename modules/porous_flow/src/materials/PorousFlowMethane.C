/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMethane.h"

template<>
InputParameters validParams<PorousFlowMethane>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addClassDescription("This Material calculates fluid properties for methane");
  return params;
}

PorousFlowMethane::PorousFlowMethane(const InputParameters & parameters) :
    PorousFlowFluidPropertiesBase(parameters),

    _density_nodal(declareProperty<Real>("PorousFlow_fluid_phase_density" + _phase)),
    _density_nodal_old(declarePropertyOld<Real>("PorousFlow_fluid_phase_density" + _phase)),
    _ddensity_nodal_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + _phase, _pressure_variable_name)),
    _ddensity_nodal_dt(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + _phase, _temperature_variable_name)),
    _density_qp(declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + _phase)),
    _ddensity_qp_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _pressure_variable_name)),
    _ddensity_qp_dt(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _temperature_variable_name)),
    _viscosity_nodal(declareProperty<Real>("PorousFlow_viscosity" + _phase)),
    _dviscosity_nodal_dt(declarePropertyDerivative<Real>("PorousFlow_viscosity" + _phase, _temperature_variable_name)),
    _Mch4(16.0425e-3)
{
}

void
PorousFlowMethane::initQpStatefulProperties()
{
  _density_nodal[_qp] = density(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp][_phase_num]);
}

void
PorousFlowMethane::computeQpProperties()
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
PorousFlowMethane::density(Real pressure, Real temperature) const
{
  return pressure * _Mch4 / (_R * (temperature + _t_c2k));
}

Real
PorousFlowMethane::dDensity_dP(Real temperature) const
{
  return _Mch4 / (_R * (temperature + _t_c2k));
}

Real
PorousFlowMethane::dDensity_dT(Real pressure, Real temperature) const
{
  const Real tk = temperature + _t_c2k;
  return - pressure * _Mch4 / (_R * tk * tk);
}

Real
PorousFlowMethane::viscosity(Real temperature) const
{
  const Real a[6] = {2.968267e-1, 3.711201e-2, 1.218298e-5, -7.02426e-8, 7.543269e-11,
    -2.7237166e-14};

  Real viscosity;
  const Real tk = temperature + _t_c2k;
  const Real tk2 = tk * tk;
  const Real tk3 = tk2 * tk;
  const Real tk4 = tk3 * tk;
  const Real tk5 = tk4 * tk;

  viscosity = a[0] + a[1] * tk + a[2] * tk2 + a[3] * tk3 + a[4] * tk4 + a[5] * tk5;

  return viscosity * 1.e-6;
}

Real
PorousFlowMethane::dViscosity_dT(Real temperature) const
{
  const Real a[6] = {2.968267e-1, 3.711201e-2, 1.218298e-5, -7.02426e-8, 7.543269e-11,
    -2.7237166e-14};

  Real dviscosity;
  const Real tk = temperature + _t_c2k;
  const Real tk2 = tk * tk;
  const Real tk3 = tk2 * tk;
  const Real tk4 = tk3 * tk;

  dviscosity = a[1] + 2.0 * a[2] * tk + 3.0 * a[3] * tk2 + 4.0 * a[4] * tk3 + 5.0 * a[5] * tk4;

  return dviscosity * 1.e-6;
}

std::vector<Real>
PorousFlowMethane::henryConstants() const
{
  return {-10.44708, 4.66491, 12.12986};
}
