/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlow1PhaseP_BW.h"
#include "libmesh/utility.h"

template <>
InputParameters
validParams<PorousFlow1PhaseP_BW>()
{
  InputParameters params = validParams<PorousFlow1PhaseP>();
  params.addRequiredRangeCheckedParam<Real>(
      "Sn",
      "Sn >= 0",
      "Low saturation.  This must be < Ss, and non-negative.  This is BW's "
      "initial effective saturation, below which effective saturation never goes "
      "in their simulations/models.  If Kn=0 then Sn is the immobile saturation.  "
      "This form of effective saturation is only correct for Kn small.");
  params.addRangeCheckedParam<Real>(
      "Ss",
      1.0,
      "Ss <= 1",
      "High saturation.  This must be > Sn and <= 1.  Effective saturation "
      "where porepressure = 0.  Effective saturation never exceeds this "
      "value in BW's simulations/models.");
  params.addRequiredRangeCheckedParam<Real>(
      "C", "C > 1", "BW's C parameter.  Must be > 1.  Typical value would be 1.05.");
  params.addRequiredRangeCheckedParam<Real>(
      "las",
      "las > 0",
      "BW's lambda_s parameter multiplied by (fluid_density * gravity).  Must be "
      "> 0.  Typical value would be 1E5");
  params.addClassDescription("Broadbridge-white form of effective saturation for negligable Kn.  "
                             "Then porepressure = -las * ((1 - th) / th - (1 / c) * Ln((C - "
                             "th)/((C - 1) * th))), for th = (Seff - Sn) / (Ss - Sn).  A Lambert-W "
                             "function must be evaluated to express Seff in terms of porepressure, "
                             "which can be expensive");
  return params;
}

PorousFlow1PhaseP_BW::PorousFlow1PhaseP_BW(const InputParameters & parameters)
  : PorousFlow1PhaseP(parameters),
    _sn(getParam<Real>("Sn")),
    _ss(getParam<Real>("Ss")),
    _c(getParam<Real>("C")),
    _las(getParam<Real>("las"))
{
  if (_ss <= _sn)
    mooseError("In BW effective saturation Sn set to ",
               _sn,
               " and Ss set to ",
               _ss,
               " but these must obey Ss > Sn");
}

Real
PorousFlow1PhaseP_BW::effectiveSaturation(Real pressure) const
{
  return PorousFlowBroadbridgeWhite::effectiveSaturation(pressure, _c, _sn, _ss, _las);
}

Real
PorousFlow1PhaseP_BW::dEffectiveSaturation_dP(Real pressure) const
{
  return PorousFlowBroadbridgeWhite::dEffectiveSaturation(pressure, _c, _sn, _ss, _las);
}

Real
PorousFlow1PhaseP_BW::d2EffectiveSaturation_dP2(Real pressure) const
{
  return PorousFlowBroadbridgeWhite::d2EffectiveSaturation(pressure, _c, _sn, _ss, _las);
}
