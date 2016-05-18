/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowDensityConstBulk.h"

template<>
InputParameters validParams<PorousFlowDensityConstBulk>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addRequiredParam<Real>("density_P0", "The density of each phase at zero porepressure");
  params.addRequiredParam<Real>("bulk_modulus", "The constant bulk modulus of each phase");
  params.addClassDescription("This Material calculates a fluid density from its porepressure, assuming constant bulk modulus for the fluid");
  return params;
}

PorousFlowDensityConstBulk::PorousFlowDensityConstBulk(const InputParameters & parameters) :
    PorousFlowFluidPropertiesBase(parameters),

    _dens0(getParam<Real>("density_P0")),
    _bulk(getParam<Real>("bulk_modulus")),
    _density_nodal(declareProperty<Real>("PorousFlow_fluid_phase_density" + _phase)),
    _density_nodal_old(declarePropertyOld<Real>("PorousFlow_fluid_phase_density" + _phase)),
    _ddensity_nodal_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + _phase, _pressure_variable_name)),
    _density_qp(declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + _phase)),
    _ddensity_qp_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _pressure_variable_name))
{
}

void
PorousFlowDensityConstBulk::initQpStatefulProperties()
{
  _density_nodal[_qp] = density(_porepressure_nodal[_qp][_phase_num]);
}

void
PorousFlowDensityConstBulk::computeQpProperties()
{
  /// Density and derivatives wrt pressure at the nodes
  _density_nodal[_qp] = density(_porepressure_nodal[_qp][_phase_num]);
  _ddensity_nodal_dp[_qp] = dDensity_dP(_porepressure_nodal[_qp][_phase_num]);

  /// Density and derivatives wrt pressure at the qps
  _density_qp[_qp] = density(_porepressure_qp[_qp][_phase_num]);
  _ddensity_qp_dp[_qp] = dDensity_dP(_porepressure_qp[_qp][_phase_num]);
}

Real
PorousFlowDensityConstBulk::density(Real pressure) const
{
  return _dens0 * std::exp(pressure / _bulk);
}

Real
PorousFlowDensityConstBulk::dDensity_dP(Real pressure) const
{
  return density(pressure) / _bulk;
}
