//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowCapillaryPressureVG.h"
#include "PorousFlowVanGenuchten.h"

registerMooseObject("PorousFlowApp", PorousFlowCapillaryPressureVG);

InputParameters
PorousFlowCapillaryPressureVG::validParams()
{
  InputParameters params = PorousFlowCapillaryPressure::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "m",
      "m >= 0 & m <= 1",
      "van Genuchten exponent m. Must be between 0 and 1, and optimally should be set to >0.5");
  params.addRequiredRangeCheckedParam<Real>(
      "alpha", "alpha > 0", "van Genuchten parameter alpha. Must be positive");
  params.addRangeCheckedParam<Real>("s_scale",
                                    1.0,
                                    "s_scale > 0.0 & s_scale <= 1.0",
                                    "CapillaryPressure = f(Seff * s_scale) - "
                                    "f(s_scale), where f is the van Genuchten "
                                    "expression.  Setting s_scale<1 is unusual "
                                    "but sometimes helps fully saturated, "
                                    "2-phase PP simulations converge as the "
                                    "zero derivative (1/f'(S=1)=0) is removed");
  params.addClassDescription("van Genuchten capillary pressure");
  return params;
}

PorousFlowCapillaryPressureVG::PorousFlowCapillaryPressureVG(const InputParameters & parameters)
  : PorousFlowCapillaryPressure(parameters),
    _m(getParam<Real>("m")),
    _alpha(getParam<Real>("alpha")),
    _s_scale(getParam<Real>("s_scale")),
    _pc_sscale(PorousFlowVanGenuchten::capillaryPressure(_s_scale, _alpha, _m, _pc_max))
{
}

Real
PorousFlowCapillaryPressureVG::capillaryPressureCurve(Real saturation, unsigned /*qp*/) const
{
  const Real seff = effectiveSaturationFromSaturation(saturation) * _s_scale;
  return PorousFlowVanGenuchten::capillaryPressure(seff, _alpha, _m, _pc_max) - _pc_sscale;
}

Real
PorousFlowCapillaryPressureVG::dCapillaryPressureCurve(Real saturation, unsigned /*qp*/) const
{
  const Real seff = effectiveSaturationFromSaturation(saturation) * _s_scale;
  return PorousFlowVanGenuchten::dCapillaryPressure(seff, _alpha, _m, _pc_max) * _dseff_ds *
         _s_scale;
}

Real
PorousFlowCapillaryPressureVG::d2CapillaryPressureCurve(Real saturation, unsigned /*qp*/) const
{
  const Real seff = effectiveSaturationFromSaturation(saturation) * _s_scale;
  return PorousFlowVanGenuchten::d2CapillaryPressure(seff, _alpha, _m, _pc_max) * _dseff_ds *
         _dseff_ds * _s_scale * _s_scale;
}

Real
PorousFlowCapillaryPressureVG::effectiveSaturation(Real pc, unsigned /*qp*/) const
{
  return (1.0 / _s_scale) *
         PorousFlowVanGenuchten::effectiveSaturation(pc - _pc_sscale, _alpha, _m);
}

Real
PorousFlowCapillaryPressureVG::dEffectiveSaturation(Real pc, unsigned /*qp*/) const
{
  return (1.0 / _s_scale) *
         PorousFlowVanGenuchten::dEffectiveSaturation(pc - _pc_sscale, _alpha, _m);
}

Real
PorousFlowCapillaryPressureVG::d2EffectiveSaturation(Real pc, unsigned /*qp*/) const
{
  return (1.0 / _s_scale) *
         PorousFlowVanGenuchten::d2EffectiveSaturation(pc - _pc_sscale, _alpha, _m);
}
