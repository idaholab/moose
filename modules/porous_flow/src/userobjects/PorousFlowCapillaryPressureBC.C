/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowCapillaryPressureBC.h"
#include "PorousFlowBrooksCorey.h"

template <>
InputParameters
validParams<PorousFlowCapillaryPressureBC>()
{
  InputParameters params = validParams<PorousFlowCapillaryPressure>();
  params.addRequiredParam<Real>("lambda", "Brooks-Corey exponent lambda");
  params.addRequiredRangeCheckedParam<Real>(
      "pe", "pe > 0", "Brooks-Corey entry pressure. Must be positive");
  params.addClassDescription("Brooks-Corey capillary pressure");
  return params;
}

PorousFlowCapillaryPressureBC::PorousFlowCapillaryPressureBC(const InputParameters & parameters)
  : PorousFlowCapillaryPressure(parameters),
    _lambda(getParam<Real>("lambda")),
    _pe(getParam<Real>("pe"))
{
}

Real
PorousFlowCapillaryPressureBC::capillaryPressureCurve(Real saturation) const
{
  Real seff = effectiveSaturationFromSaturation(saturation);
  return PorousFlowBrooksCorey::capillaryPressure(seff, _pe, _lambda, _pc_max);
}

Real
PorousFlowCapillaryPressureBC::dCapillaryPressureCurve(Real saturation) const
{
  Real seff = effectiveSaturationFromSaturation(saturation);
  return PorousFlowBrooksCorey::dCapillaryPressure(seff, _pe, _lambda, _pc_max) * _dseff_ds;
}

Real
PorousFlowCapillaryPressureBC::d2CapillaryPressureCurve(Real saturation) const
{
  Real seff = effectiveSaturationFromSaturation(saturation);
  return PorousFlowBrooksCorey::d2CapillaryPressure(seff, _pe, _lambda, _pc_max) * _dseff_ds *
         _dseff_ds;
}

Real
PorousFlowCapillaryPressureBC::effectiveSaturation(Real pc) const
{
  return PorousFlowBrooksCorey::effectiveSaturation(pc, _pe, _lambda);
}

Real
PorousFlowCapillaryPressureBC::dEffectiveSaturation(Real pc) const
{
  return PorousFlowBrooksCorey::dEffectiveSaturation(pc, _pe, _lambda);
}

Real
PorousFlowCapillaryPressureBC::d2EffectiveSaturation(Real pc) const
{
  return PorousFlowBrooksCorey::d2EffectiveSaturation(pc, _pe, _lambda);
}
