//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowCapillaryPressureBW.h"
#include "PorousFlowBroadbridgeWhite.h"

registerMooseObject("PorousFlowApp", PorousFlowCapillaryPressureBW);

InputParameters
PorousFlowCapillaryPressureBW::validParams()
{
  InputParameters params = PorousFlowCapillaryPressure::validParams();
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
  params.addClassDescription("Broadbridge and White capillary pressure for negligable Kn");
  return params;
}

PorousFlowCapillaryPressureBW::PorousFlowCapillaryPressureBW(const InputParameters & parameters)
  : PorousFlowCapillaryPressure(parameters),
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

  // Set _log_ext to false as no capillary pressure curves are implmented in this class
  _log_ext = false;
}

Real
PorousFlowCapillaryPressureBW::capillaryPressureCurve(Real /*saturation*/, unsigned /*qp*/) const
{
  mooseError("PorousFlowCapillaryPressureBW::capillaryPressure not implemented");
  return 0.0;
}

Real
PorousFlowCapillaryPressureBW::dCapillaryPressureCurve(Real /*saturation*/, unsigned /*qp*/) const
{
  mooseError("PorousFlowCapillaryPressureBW::dCapillaryPressure not implemented");
  return 0.0;
}

Real
PorousFlowCapillaryPressureBW::d2CapillaryPressureCurve(Real /*saturation*/, unsigned /*qp*/) const
{
  mooseError("PorousFlowCapillaryPressureBW::d2CapillaryPressure not implemented");
  return 0.0;
}

Real
PorousFlowCapillaryPressureBW::effectiveSaturation(Real pc, unsigned /*qp*/) const
{
  return PorousFlowBroadbridgeWhite::effectiveSaturation(pc, _c, _sn, _ss, _las);
}

Real
PorousFlowCapillaryPressureBW::dEffectiveSaturation(Real pc, unsigned /*qp*/) const
{
  return PorousFlowBroadbridgeWhite::dEffectiveSaturation(pc, _c, _sn, _ss, _las);
}

Real
PorousFlowCapillaryPressureBW::d2EffectiveSaturation(Real pc, unsigned /*qp*/) const
{
  return PorousFlowBroadbridgeWhite::d2EffectiveSaturation(pc, _c, _sn, _ss, _las);
}
