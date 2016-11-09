/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlow2PhasePP_VG.h"

template<>
InputParameters validParams<PorousFlow2PhasePP_VG>()
{
  InputParameters params = validParams<PorousFlow2PhasePP>();
  params.addRequiredRangeCheckedParam<Real>("al", "al > 0", "van-Genuchten alpha parameter.  Must be positive.  effectiveSaturation = (1 + (-al*P)^(1/(1-m)))^(-m), where P = phase0_porepressure - phase1_porepressure <= 0");
  params.addRequiredRangeCheckedParam<Real>("m", "m > 0 & m < 1", "van-Genuchten m parameter.  Must be between 0 and 1, and optimally should be set to >0.5   EffectiveSaturation = (1 + (-al*p)^(1/(1-m)))^(-m)");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations in a 2-phase isothermal situation, and derivatives of these with respect to the PorousFlowVariables.  A van-Genuchten capillary suction function is assumed");
  return params;
}

PorousFlow2PhasePP_VG::PorousFlow2PhasePP_VG(const InputParameters & parameters) :
    PorousFlow2PhasePP(parameters),
    _al(getParam<Real>("al")),
    _m(getParam<Real>("m"))
{
}

Real
PorousFlow2PhasePP_VG::effectiveSaturation(Real pressure) const
{
  return PorousFlowVanGenuchten::seff(pressure, _al, _m);
}

Real
PorousFlow2PhasePP_VG::dEffectiveSaturation_dP(Real pressure) const
{
  return PorousFlowVanGenuchten::dseff(pressure, _al, _m);
}

Real
PorousFlow2PhasePP_VG::d2EffectiveSaturation_dP2(Real pressure) const
{
  return PorousFlowVanGenuchten::d2seff(pressure, _al, _m);
}
