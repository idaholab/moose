/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterialDensityConstBulk.h"
#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowMaterialDensityConstBulk>()
{
  InputParameters params = validParams<PorousFlowMaterialFluidPropertiesBase>();
  params.addRequiredParam<Real>("density0", "The density of each phase at zero porepressure");
  params.addRequiredParam<Real>("bulk_modulus", "The constant bulk modulus of each phase");
  params.addClassDescription("This Material calculates a fluid density from its porepressure, assuming constant bulk modulus for the fluid");
  return params;
}

PorousFlowMaterialDensityConstBulk::PorousFlowMaterialDensityConstBulk(const InputParameters & parameters) :
    PorousFlowMaterialFluidPropertiesBase(parameters),

    _dens0(getParam<Real>("density0")),
    _bulk(getParam<Real>("bulk_modulus")),
    _density(declareProperty<Real>("PorousFlow_fluid_phase_density" + Moose::stringify(_phase_num))),
    _density_old(declarePropertyOld<Real>("PorousFlow_fluid_phase_density" + Moose::stringify(_phase_num))),
    _ddensity_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + Moose::stringify(_phase_num), _pressure_variable_name)),
    _density_qp(declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + Moose::stringify(_phase_num))),
    _ddensity_qp_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + Moose::stringify(_phase_num), _pressure_variable_name))
{
}

void
PorousFlowMaterialDensityConstBulk::initQpStatefulProperties()
{
  _density[_qp] = _dens0 * std::exp(_porepressure[_qp][_phase_num]/_bulk);
}

void
PorousFlowMaterialDensityConstBulk::computeQpProperties()
{
  /// Density and derivatives wrt pressure at the nodes
  _density[_qp] = density(_porepressure[_qp][_phase_num]);
  _ddensity_dp[_qp] = dDensity_dP(_porepressure[_qp][_phase_num]);

  /// Density and derivatives wrt pressure at the qps
  _density_qp[_qp] = density(_porepressure_qp[_qp][_phase_num]);
  _ddensity_qp_dp[_qp] = dDensity_dP(_porepressure_qp[_qp][_phase_num]);
}

Real
PorousFlowMaterialDensityConstBulk::density(Real pressure) const
{
  return _dens0 * std::exp(pressure / _bulk);
}

Real
PorousFlowMaterialDensityConstBulk::dDensity_dP(Real pressure) const
{
  return density(pressure) / _bulk;
}
