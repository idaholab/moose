/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlow2PhasePP_RSC.h"

template <>
InputParameters
validParams<PorousFlow2PhasePP_RSC>()
{
  InputParameters params = validParams<PorousFlow2PhasePP>();
  params.addParam<Real>("oil_viscosity",
                        "Viscosity of oil (gas) phase.  It is assumed this is "
                        "double the water-phase viscosity.  (Note that this "
                        "effective saturation is mostly useful for 2-phase, not "
                        "single-phase.)");
  params.addParam<Real>("scale_ratio",
                        "This is porosity / permeability / beta^2, where beta may "
                        "be chosen by the user.  It has dimensions [time]");
  params.addParam<Real>("shift", "effective saturation is a function of (Pc - shift)");
  params.addClassDescription("Rogers-Stallybrass-Clements version of effective saturation for the "
                             "water phase, valid for residual saturations = 0, and viscosityOil = "
                             "2 * viscosityWater.  seff_water = 1 / sqrt(1 + exp((Pc - shift) / "
                             "scale)), where scale = 0.25 * scale_ratio * oil_viscosity.");
  return params;
}

PorousFlow2PhasePP_RSC::PorousFlow2PhasePP_RSC(const InputParameters & parameters)
  : PorousFlow2PhasePP(parameters),
    _oil_viscosity(getParam<Real>("oil_viscosity")),
    _scale_ratio(getParam<Real>("scale_ratio")),
    _shift(getParam<Real>("shift")),
    _scale(0.25 * _scale_ratio * _oil_viscosity)
{
}

Real
PorousFlow2PhasePP_RSC::effectiveSaturation(Real pressure) const
{
  return PorousFlowRogersStallybrassClements::effectiveSaturation(-pressure, _shift, _scale);
}

Real
PorousFlow2PhasePP_RSC::dEffectiveSaturation_dP(Real pressure) const
{
  return -PorousFlowRogersStallybrassClements::dEffectiveSaturation(-pressure, _shift, _scale);
}

Real
PorousFlow2PhasePP_RSC::d2EffectiveSaturation_dP2(Real pressure) const
{
  return PorousFlowRogersStallybrassClements::d2EffectiveSaturation(-pressure, _shift, _scale);
}
