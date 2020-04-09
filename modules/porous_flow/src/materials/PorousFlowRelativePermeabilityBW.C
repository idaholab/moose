//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowRelativePermeabilityBW.h"

registerMooseObject("PorousFlowApp", PorousFlowRelativePermeabilityBW);

InputParameters
PorousFlowRelativePermeabilityBW::validParams()
{
  InputParameters params = PorousFlowRelativePermeabilityBase::validParams();
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
      "Kn", "Kn >= 0", "Low relative permeability.  This must be < Ks, and non-negative.");
  params.addRequiredRangeCheckedParam<Real>(
      "Ks", "Ks <= 1", "High relative permeability.  This must be > Kn and less than unity");
  params.addRequiredRangeCheckedParam<Real>(
      "C", "C > 1", "BW's C parameter.  Must be > 1.  Typical value would be 1.05.");
  params.addClassDescription("Broadbridge-White form of relative permeability");
  return params;
}

PorousFlowRelativePermeabilityBW::PorousFlowRelativePermeabilityBW(
    const InputParameters & parameters)
  : PorousFlowRelativePermeabilityBase(parameters),
    _sn(getParam<Real>("Sn")),
    _ss(getParam<Real>("Ss")),
    _kn(getParam<Real>("Kn")),
    _ks(getParam<Real>("Ks")),
    _c(getParam<Real>("C"))
{
  if (_ss <= _sn)
    mooseError("In BW relative permeability Sn set to ",
               _sn,
               " and Ss set to ",
               _ss,
               " but these must obey Ss > Sn");
  if (_ks <= _kn)
    mooseError("In BW relative permeability Kn set to ",
               _kn,
               " and Ks set to ",
               _ks,
               " but these must obey Ks > Kn");
}

Real
PorousFlowRelativePermeabilityBW::relativePermeability(Real seff) const
{
  return PorousFlowBroadbridgeWhite::relativePermeability(seff, _c, _sn, _ss, _kn, _ks);
}

Real
PorousFlowRelativePermeabilityBW::dRelativePermeability(Real seff) const
{
  return PorousFlowBroadbridgeWhite::dRelativePermeability(seff, _c, _sn, _ss, _kn, _ks);
}
