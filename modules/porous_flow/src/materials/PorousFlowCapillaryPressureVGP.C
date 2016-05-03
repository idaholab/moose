/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowCapillaryPressureVGP.h"
#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowCapillaryPressureVGP>()
{
  InputParameters params = validParams<PorousFlowCapillaryPressureBase>();
  params.addRequiredRangeCheckedParam<Real>("al", "al > 0", "van Genuchten alpha parameter.  Must be positive.  effectiveSaturation = (1 + (-al*P)^(1/(1-m)))^(-m), where P = phase0_porepressure - phase1_porepressure <= 0");
  params.addRequiredRangeCheckedParam<Real>("m", "m > 0 & m < 1", "van Genuchten m parameter.  Must be between 0 and 1, and optimally should be set to >0.5   EffectiveSaturation = (1 + (-al*p)^(1/(1-m)))^(-m)");
  params.addClassDescription("This Material provides a van Genuchten effective saturation as a function of pore pressure");
  return params;
}

PorousFlowCapillaryPressureVGP::PorousFlowCapillaryPressureVGP(const InputParameters & parameters) :
    PorousFlowCapillaryPressureBase(parameters),
    _pressure_variable_name(_dictator_UO.pressureVariableNameDummy()),
    _porepressure_nodal(getMaterialProperty<std::vector<Real> >("PorousFlow_porepressure_nodal")),
    _porepressure_qp(getMaterialProperty<std::vector<Real> >("PorousFlow_porepressure_qp")),
    _dcapillary_pressure_nodal_dp(declarePropertyDerivative<Real>("PorousFlow_capillary_pressure_nodal" + Moose::stringify(_phase_num), _pressure_variable_name)),
    _d2capillary_pressure_nodal_dp2(declarePropertyDerivative<Real>("PorousFlow_capillary_pressure_nodal" + Moose::stringify(_phase_num), _pressure_variable_name, _pressure_variable_name)),
    _al(getParam<Real>("al")),
    _m(getParam<Real>("m"))
{
}

void
PorousFlowCapillaryPressureVGP::computeQpProperties()
{
  /// Capillary pressure (really effecive saturation) and derivatives wrt phase pore pressure at the nodes
  _capillary_pressure_nodal[_qp] = effectiveSaturation(_porepressure_nodal[_qp][_phase_num]);
  _dcapillary_pressure_nodal_ds[_qp] = dEffectiveSaturation(_porepressure_nodal[_qp][_phase_num]);
  _d2capillary_pressure_nodal_ds2[_qp] = d2EffectiveSaturation(_porepressure_nodal[_qp][_phase_num]);

  /// Capillary pressure (really effecive saturation) and derivatives wrt phase pore pressure at the qps
  _capillary_pressure_qp[_qp] = effectiveSaturation(_porepressure_qp[_qp][_phase_num]);
  _dcapillary_pressure_qp_ds[_qp] = dEffectiveSaturation(_porepressure_qp[_qp][_phase_num]);
  _d2capillary_pressure_qp_ds2[_qp] = d2EffectiveSaturation(_porepressure_qp[_qp][_phase_num]);
}

Real
PorousFlowCapillaryPressureVGP::effectiveSaturation(Real pressure) const
{
  Real n, seff;

  if (pressure >= 0.0)
    return 1.0;
  else
  {
    n = 1.0 / (1.0 - _m);
    seff = 1.0 + std::pow(- _al * pressure, n);
    return std::pow(seff, - _m);
  }
}

Real
PorousFlowCapillaryPressureVGP::dEffectiveSaturation(Real pressure) const
{
  if (pressure >= 0.0)
    return 0.0;
  else
  {
    Real n = 1.0 / (1.0 - _m);
    Real inner = 1.0 + std::pow(- _al * pressure, n);
    Real dinner_dp = - n * _al * std::pow(- _al * pressure, n - 1.0);
    Real dseff_dp = - _m * std::pow(inner, - _m - 1.0) * dinner_dp;
    return dseff_dp;
  }
}

Real
PorousFlowCapillaryPressureVGP::d2EffectiveSaturation(Real pressure) const
{
  if (pressure >= 0.0)
    return 0.0;
  else
  {
    Real n = 1.0 / (1.0 - _m);
    Real inner = 1.0 + std::pow(- _al * pressure, n);
    Real dinner_dp = - n * _al * std::pow(- _al * pressure, n - 1.0);
    Real d2inner_dp2 = n * (n - 1.0) * _al * _al * std::pow(- _al * pressure, n - 2.0);
    Real d2seff_dp2 = _m * (_m + 1.0) * std::pow(inner, - _m - 2.0) * std::pow(dinner_dp, 2.0) - _m * std::pow(inner, - _m - 1.0) * d2inner_dp2;
    return d2seff_dp2;
  }
}
