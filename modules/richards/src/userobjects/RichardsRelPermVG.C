/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "RichardsRelPermVG.h"
#include "libmesh/utility.h"

template <>
InputParameters
validParams<RichardsRelPermVG>()
{
  InputParameters params = validParams<RichardsRelPerm>();
  params.addRequiredRangeCheckedParam<Real>(
      "simm",
      "simm >= 0 & simm < 1",
      "Immobile saturation.  Must be between 0 and 1.  Define s = "
      "(seff - simm)/(1 - simm).  Then relperm = s^(1/2) * (1 - (1 "
      "- s^(1/m))^m)^2");
  params.addRequiredRangeCheckedParam<Real>(
      "m",
      "m > 0 & m < 1",
      "van-Genuchten m parameter.  Must be between 0 and 1, and optimally "
      "should be set >0.5.  Define s = (seff - simm)/(1 - simm).  Then "
      "relperm = s^(1/2) * (1 - (1 - s^(1/m))^m)^2");
  params.addClassDescription("VG form of relative permeability.  Define s = (seff - simm)/(1 - "
                             "simm).  Then relperm = s^(1/2) * (1 - (1 - s^(1/m))^m)^2, if s>0, "
                             "and relperm=0 otherwise");
  return params;
}

RichardsRelPermVG::RichardsRelPermVG(const InputParameters & parameters)
  : RichardsRelPerm(parameters), _simm(getParam<Real>("simm")), _m(getParam<Real>("m"))
{
}

Real
RichardsRelPermVG::relperm(Real seff) const
{
  if (seff >= 1.0)
    return 1.0;

  if (seff <= _simm)
    return 0.0;

  Real s_internal = (seff - _simm) / (1.0 - _simm);
  Real krel = std::sqrt(s_internal) *
              Utility::pow<2>(1.0 - std::pow(1.0 - std::pow(s_internal, 1.0 / _m), _m));

  // bound, just in case
  if (krel < 0.0)
    krel = 0.0;
  if (krel > 1.0)
    krel = 1.0;

  return krel;
}

Real
RichardsRelPermVG::drelperm(Real seff) const
{
  if (seff >= 1.0)
    return 0.0;

  if (seff <= _simm)
    return 0.0;

  Real s_internal = (seff - _simm) / (1.0 - _simm);
  Real tmp = 1.0 - std::pow(s_internal, 1.0 / _m);
  Real tmpp = -1.0 / _m * std::pow(s_internal, 1.0 / _m - 1.0);
  Real tmp2 = 1.0 - std::pow(tmp, _m);
  Real tmp2p = -_m * std::pow(tmp, _m - 1.0) * tmpp;
  // Real krel = std::sqrt(s_internal)*std::pow(tmp2, 2);
  Real krelp =
      0.5 * std::pow(s_internal, -0.5) * tmp2 * tmp2 + 2.0 * std::sqrt(s_internal) * tmp2 * tmp2p;
  return krelp / (1.0 - _simm);
}

Real
RichardsRelPermVG::d2relperm(Real seff) const
{
  if (seff >= 1.0)
    return 0.0;

  if (seff <= _simm)
    return 0.0;

  Real s_internal = (seff - _simm) / (1.0 - _simm);
  Real tmp = 1.0 - std::pow(s_internal, 1.0 / _m);
  Real tmpp = -1.0 / _m * std::pow(s_internal, 1.0 / _m - 1.0);
  Real tmppp = -1.0 / _m * (1.0 / _m - 1.0) * std::pow(s_internal, 1.0 / _m - 2);
  Real tmp2 = 1.0 - std::pow(tmp, _m);
  Real tmp2p = -_m * std::pow(tmp, _m - 1.0) * tmpp;
  Real tmp2pp = -_m * (_m - 1.0) * std::pow(tmp, _m - 2.0) * tmpp * tmpp -
                _m * std::pow(tmp, _m - 1.0) * tmppp;
  // Real krel = std::sqrt(s_internal)*std::pow(tmp2, 2);
  // Real krelp = 0.5 * std::pow(s_internal, -0.5)*std::pow(tmp2, 2) +
  // 2*std::sqrt(s_internal)*tmp2*tmp2p;
  Real krelpp = -0.25 * std::pow(s_internal, -1.5) * tmp2 * tmp2 +
                2.0 * 0.5 * std::pow(s_internal, -0.5) * 2.0 * tmp2 * tmp2p +
                2.0 * std::sqrt(s_internal) * (tmp2p * tmp2p + tmp2 * tmp2pp);

  return krelpp / Utility::pow<2>(1.0 - _simm);
}
