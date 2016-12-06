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
  params.addParam<bool>("at_nodes", false, "Material properties will be calculated at nodes rather than the usual quadpoints");
  params.addClassDescription("This Material calculates a fluid density from its porepressure, assuming constant bulk modulus for the fluid");
  return params;
}

PorousFlowDensityConstBulk::PorousFlowDensityConstBulk(const InputParameters & parameters) :
    PorousFlowFluidPropertiesBase(parameters),

    _dens0(getParam<Real>("density_P0")),
    _bulk(getParam<Real>("bulk_modulus")),
    _density_nodal(getParam<bool>("at_nodes") ? &declareProperty<Real>("PorousFlow_fluid_phase_density" + _phase) : nullptr),
    _density_nodal_old(getParam<bool>("at_nodes") ? &declarePropertyOld<Real>("PorousFlow_fluid_phase_density" + _phase) : nullptr),
    _ddensity_nodal_dp(getParam<bool>("at_nodes") ? &declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + _phase, _pressure_variable_name) : nullptr),
    _ddensity_nodal_dt(getParam<bool>("at_nodes") ? &declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + _phase, _temperature_variable_name) : nullptr),
    _density_qp(!getParam<bool>("at_nodes") ? &declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + _phase) : nullptr),
    _ddensity_qp_dp(!getParam<bool>("at_nodes") ? &declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _pressure_variable_name) : nullptr)
{
  _nodal_material = getParam<bool>("at_nodes");
}

void
PorousFlowDensityConstBulk::initQpStatefulProperties()
{
  if (_nodal_material)
    (*_density_nodal)[_qp] = density(_porepressure_nodal[_qp][_phase_num]);
}

void
PorousFlowDensityConstBulk::computeQpProperties()
{
  if (_nodal_material)
  {
    /// Density and derivatives wrt pressure at the nodes
    (*_density_nodal)[_qp] = density(_porepressure_nodal[_qp][_phase_num]);
    (*_ddensity_nodal_dp)[_qp] = dDensity_dP(_porepressure_nodal[_qp][_phase_num]);
    (*_ddensity_nodal_dt)[_qp] = 0.0;
  }
  else
  {
    /// Density and derivatives wrt pressure at the qps
    (*_density_qp)[_qp] = density(_porepressure_qp[_qp][_phase_num]);
    (*_ddensity_qp_dp)[_qp] = dDensity_dP(_porepressure_qp[_qp][_phase_num]);
  }
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
