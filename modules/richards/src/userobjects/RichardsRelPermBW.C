//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  "Broadbridge-White" form of relative permeability (P Broadbridge and I White ``Constant rate
//  rainfall infiltration: A versatile nonlinear model 1. Analytic Solution'', Water Resources
//  Research 24 (1988) 145-154)
//
#include "RichardsRelPermBW.h"
#include "libmesh/utility.h"

registerMooseObject("RichardsApp", RichardsRelPermBW);

InputParameters
RichardsRelPermBW::validParams()
{
  InputParameters params = RichardsRelPerm::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "Sn",
      "Sn >= 0",
      "Low saturation.  This must be < Ss, and non-negative.  This is BW's "
      "initial effective saturation, below which effective saturation never goes "
      "in their simulations/models.  If Kn=0 then Sn is the immobile saturation.");
  params.addRangeCheckedParam<Real>(
      "Ss",
      1.0,
      "Ss <= 1",
      "High saturation.  This must be > Sn and <= 1.  Effective saturation "
      "where porepressure = 0.  Effective saturation never exceeds this "
      "value in BW's simulations/models.");
  params.addRangeCheckedParam<Real>(
      "Kn", 0.0, "Kn >= 0", "Relative permeability at Seff = Sn.  Must be < Ks");
  params.addRangeCheckedParam<Real>(
      "Ks", 1.0, "Ks <= 1", "Relative permeability at Seff = Ss.  Must be > Kn");
  params.addRequiredRangeCheckedParam<Real>(
      "C",
      "C > 1",
      "BW's C parameter.  Must be > 1.   Define s = (seff - Sn)/(Ss - Sn).  Then "
      "relperm = Kn + s^2(c-1)(Kn-Ks)/(c-s) if 0<s<1, otherwise relperm = Kn if "
      "s<=0, otherwise relperm = Ks if s>=1.");
  params.addClassDescription("Broadbridge-White form of relative permeability.  Define s = (seff - "
                             "Sn)/(Ss - Sn).  Then relperm = Kn + s^2(c-1)(Kn-Ks)/(c-s) if 0<s<1, "
                             "otherwise relperm = Kn if s<=0, otherwise relperm = Ks if s>=1.");
  return params;
}

RichardsRelPermBW::RichardsRelPermBW(const InputParameters & parameters)
  : RichardsRelPerm(parameters),
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
  _coef = (_ks - _kn) * (_c - 1); // shorthand coefficient
}

Real
RichardsRelPermBW::relperm(Real seff) const
{
  if (seff <= _sn)
    return _kn;

  if (seff >= _ss)
    return _ks;

  const Real s_internal = (seff - _sn) / (_ss - _sn);
  const Real krel = _kn + _coef * Utility::pow<2>(s_internal) / (_c - s_internal);
  return krel;
}

Real
RichardsRelPermBW::drelperm(Real seff) const
{
  if (seff <= _sn)
    return 0.0;

  if (seff >= _ss)
    return 0.0;

  const Real s_internal = (seff - _sn) / (_ss - _sn);
  const Real krelp = _coef * (2.0 * s_internal / (_c - s_internal) +
                              Utility::pow<2>(s_internal) / Utility::pow<2>(_c - s_internal));
  return krelp / (_ss - _sn);
}

Real
RichardsRelPermBW::d2relperm(Real seff) const
{
  if (seff <= _sn)
    return 0.0;

  if (seff >= _ss)
    return 0.0;

  const Real s_internal = (seff - _sn) / (_ss - _sn);
  const Real krelpp =
      _coef * (2.0 / (_c - s_internal) + 4.0 * s_internal / Utility::pow<2>(_c - s_internal) +
               2.0 * Utility::pow<2>(s_internal) / Utility::pow<3>(_c - s_internal));
  return krelpp / Utility::pow<2>(_ss - _sn);
}
