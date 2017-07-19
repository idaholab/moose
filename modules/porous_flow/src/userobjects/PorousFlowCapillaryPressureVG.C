/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowCapillaryPressureVG.h"
#include "PorousFlowVanGenuchten.h"

template <>
InputParameters
validParams<PorousFlowCapillaryPressureVG>()
{
  InputParameters params = validParams<PorousFlowCapillaryPressure>();
  params.addRequiredRangeCheckedParam<Real>(
      "m",
      "m >= 0 & m <= 1",
      "van Genuchten exponent m. Must be between 0 and 1, and optimally should be set to >0.5");
  params.addRequiredRangeCheckedParam<Real>(
      "alpha", "alpha > 0", "van Genuchten parameter alpha. Must be positive");
  params.addClassDescription("van Genuchten capillary pressure");
  return params;
}

PorousFlowCapillaryPressureVG::PorousFlowCapillaryPressureVG(const InputParameters & parameters)
  : PorousFlowCapillaryPressure(parameters),
    _m(getParam<Real>("m")),
    _alpha(getParam<Real>("alpha"))
{
}

Real
PorousFlowCapillaryPressureVG::capillaryPressureCurve(Real saturation) const
{
  Real seff = effectiveSaturationFromSaturation(saturation);
  return PorousFlowVanGenuchten::capillaryPressure(seff, _alpha, _m, _pc_max);
}

Real
PorousFlowCapillaryPressureVG::dCapillaryPressureCurve(Real saturation) const
{
  Real seff = effectiveSaturationFromSaturation(saturation);
  return PorousFlowVanGenuchten::dCapillaryPressure(seff, _alpha, _m, _pc_max) * _dseff_ds;
}

Real
PorousFlowCapillaryPressureVG::d2CapillaryPressureCurve(Real saturation) const
{
  Real seff = effectiveSaturationFromSaturation(saturation);
  return PorousFlowVanGenuchten::d2CapillaryPressure(seff, _alpha, _m, _pc_max) * _dseff_ds *
         _dseff_ds;
}

Real
PorousFlowCapillaryPressureVG::effectiveSaturation(Real pc) const
{
  return PorousFlowVanGenuchten::effectiveSaturation(pc, _alpha, _m);
}

Real
PorousFlowCapillaryPressureVG::dEffectiveSaturation(Real pc) const
{
  return PorousFlowVanGenuchten::dEffectiveSaturation(pc, _alpha, _m);
}

Real
PorousFlowCapillaryPressureVG::d2EffectiveSaturation(Real pc) const
{
  return PorousFlowVanGenuchten::d2EffectiveSaturation(pc, _alpha, _m);
}
