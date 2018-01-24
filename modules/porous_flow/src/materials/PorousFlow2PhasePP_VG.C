//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlow2PhasePP_VG.h"

template <>
InputParameters
validParams<PorousFlow2PhasePP_VG>()
{
  InputParameters params = validParams<PorousFlow2PhasePP>();
  params.addRequiredRangeCheckedParam<Real>(
      "al", "al > 0", "van Genuchten parameter alpha.  Must be positive");
  params.addRequiredRangeCheckedParam<Real>(
      "m",
      "m > 0 & m < 1",
      "van Genuchten exponent m.  Must be between 0 and 1, and optimally should be set to >0.5");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations "
                             "in a 2-phase isothermal situation, and derivatives of these with "
                             "respect to the PorousFlowVariables. Calculates the 1 porepressure "
                             "and the 1 saturation in a 1-phase isothermal situation, and "
                             "derivatives of these with respect to the PorousFlowVariables.  A van "
                             "Genuchten effective saturation (1 + (-al * p)^(1 / (1 - m)))^(-m) is "
                             "assumed, where p = phase0_porepressure - phase1_porepressure <= 0");
  return params;
}

PorousFlow2PhasePP_VG::PorousFlow2PhasePP_VG(const InputParameters & parameters)
  : PorousFlow2PhasePP(parameters), _al(getParam<Real>("al")), _m(getParam<Real>("m"))
{
  mooseDeprecated("PorousFlow2PhasePP_VG is deprecated. Please use PorousFlow2PhasePP and a "
                  "PorousFlowCapillaryPressureVG UserObject instead");
}

Real
PorousFlow2PhasePP_VG::effectiveSaturation(Real pressure) const
{
  return PorousFlowVanGenuchten::effectiveSaturation(pressure, _al, _m);
}

Real
PorousFlow2PhasePP_VG::dEffectiveSaturation_dP(Real pressure) const
{
  return PorousFlowVanGenuchten::dEffectiveSaturation(pressure, _al, _m);
}

Real
PorousFlow2PhasePP_VG::d2EffectiveSaturation_dP2(Real pressure) const
{
  return PorousFlowVanGenuchten::d2EffectiveSaturation(pressure, _al, _m);
}
