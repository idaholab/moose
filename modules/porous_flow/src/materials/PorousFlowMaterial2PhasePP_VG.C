/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMaterial2PhasePP_VG.h"

template<>
InputParameters validParams<PorousFlowMaterial2PhasePP_VG>()
{
  InputParameters params = validParams<PorousFlowVariableBase>();

  params.addRequiredCoupledVar("phase0_porepressure", "Variable that is the porepressure of phase 0 (eg, the water phase).  It will be <= phase1_porepressure.");
  params.addRequiredCoupledVar("phase1_porepressure", "Variable that is the porepressure of phase 1 (eg, the gas phase)");
  params.addRequiredRangeCheckedParam<Real>("al", "al > 0", "van-Genuchten alpha parameter.  Must be positive.  effectiveSaturation = (1 + (-al*P)^(1/(1-m)))^(-m), where P = phase0_porepressure - phase1_porepressure <= 0");
  params.addRequiredRangeCheckedParam<Real>("m", "m > 0 & m < 1", "van-Genuchten m parameter.  Must be between 0 and 1, and optimally should be set to >0.5   EffectiveSaturation = (1 + (-al*p)^(1/(1-m)))^(-m)");
  params.addClassDescription("This Material calculates the 2 porepressures and the 2 saturations in a 2-phase isothermal situation, and derivatives of these with respect to the PorousFlowVariables.  A van-Genuchten capillary suction function is assumed");
  return params;
}

PorousFlowMaterial2PhasePP_VG::PorousFlowMaterial2PhasePP_VG(const InputParameters & parameters) :
    PorousFlowMaterial2PhasePP(parameters),

    _al(getParam<Real>("al")),
    _m(getParam<Real>("m"))
{
  if (_dictator_UO.numPhases() != 2)
    mooseError("The Dictator announces that the number of phases is " << _dictator_UO.numPhases() << " whereas PorousFlowMaterial2PhasePP_VG can only be used for 2-phase simulation.  When you have an efficient government, you have a dictatorship.");
}

Real
PorousFlowMaterial2PhasePP_VG::effectiveSaturation(Real pressure) const
{
  return PorousFlowEffectiveSaturationVG::seff(pressure, _al, _m);
}

Real
PorousFlowMaterial2PhasePP_VG::dEffectiveSaturation_dP(Real pressure) const
{
  return PorousFlowEffectiveSaturationVG::dseff(pressure, _al, _m);
}

Real
PorousFlowMaterial2PhasePP_VG::d2EffectiveSaturation_dP2(Real pressure) const
{
  return PorousFlowEffectiveSaturationVG::d2seff(pressure, _al, _m);
}
