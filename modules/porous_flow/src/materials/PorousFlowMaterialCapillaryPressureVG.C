/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMaterialCapillaryPressureVG.h"

template<>
InputParameters validParams<PorousFlowMaterialCapillaryPressureVG>()
{
  InputParameters params = validParams<PorousFlowMaterialCapillaryPressureBase>();
  params.addRequiredRangeCheckedParam<Real>("sat_lr", "sat_lr >= 0 & sat_lr <= 1", "Liquid residual saturation.  Must be between 0 and 1");
  params.addRequiredRangeCheckedParam<Real>("sat_ls", "sat_ls >= 0 & sat_ls <= 1", "Liquid fully saturated saturation.  Must be between 0 and 1");
  params.addRequiredRangeCheckedParam<Real>("m", "m >= 0 & m <= 1", "van Genuchten exponent m");
  params.addRequiredParam<Real>("pc_max", "Maximum capillary pressure");
  params.addRequiredParam<Real>("p0", "Capillary pressure coefficient P0");
  params.addClassDescription("This Material provides a van Genuchten capillary pressure as a function of phase saturation");
  return params;
}

PorousFlowMaterialCapillaryPressureVG::PorousFlowMaterialCapillaryPressureVG(const InputParameters & parameters) :
  PorousFlowMaterialCapillaryPressureBase(parameters),
  _sat_lr(getParam<Real>("sat_lr")),
  _sat_ls(getParam<Real>("sat_ls")),
  _m(getParam<Real>("m")),
  _pc_max(getParam<Real>("pc_max")),
  _p0(getParam<Real>("p0"))
{
}

void
PorousFlowMaterialCapillaryPressureVG::computeQpProperties()
{
  /// Capillary pressure and derivatives wrt phase saturation at the nodes
  _capillary_pressure_nodal[_qp] = _pc_max * (1.0 - _saturation_nodal[_qp][_phase_num]);
  _dcapillary_pressure_nodal_ds[_qp] = - _pc_max;
  _d2capillary_pressure_nodal_ds2[_qp] = 0.0;

  /// Capillary pressure and derivatives wrt phase saturation at the qps
  _capillary_pressure_qp[_qp] = _pc_max * (1.0 - _saturation_qp[_qp][_phase_num]);
  _dcapillary_pressure_qp_ds[_qp] = - _pc_max;
  _d2capillary_pressure_qp_ds2[_qp] = 0.0;
}

Real
PorousFlowMaterialCapillaryPressureVG::effectiveSaturation(Real saturation) const
{
  return (saturation - _sat_lr)/(_sat_ls - _sat_lr);
}

Real
PorousFlowMaterialCapillaryPressureVG::capillaryPressure(Real saturation) const
{
  Real sat_eff = effectiveSaturation(saturation);
  Real cp = _p0 * std::pow(std::pow(sat_eff, -1.0/_m) - 1.0, 1.0 - _m);

  /// Return the max of this and _cp_max
  return std::max(cp, _pc_max);
}

Real
PorousFlowMaterialCapillaryPressureVG::dCapillaryPressure(Real saturation) const
{
  Real sat_eff = effectiveSaturation(saturation);

  return _p0 * (1.0 - _m) * std::pow(std::pow(sat_eff, -1.0 / _m) - 1.0, -_m) * std::pow(sat_eff, -1.0 - 1.0 / _m) / (_m * (_sat_ls - _sat_lr));
}

Real
PorousFlowMaterialCapillaryPressureVG::d2CapillaryPressure(Real saturation) const
{
  Real d2cp;
  Real a;
  Real sat_eff = effectiveSaturation(saturation);

  a = std::pow(sat_eff, -1.0 / _m) - 1.0;
  d2cp = std::pow(a, -1.0 - _m) * std::pow(sat_eff, -2.0 - 2.0 / _m) - ((1.0 + _m) / _m) *std::pow(a, - _m) *
    std::pow(sat_eff, -1.0 / _m - 2.0);
  d2cp *= _p0 * (1.0 - _m) / _m / (_sat_ls - _sat_lr) / (_sat_ls - _sat_lr);

  return d2cp;
}
