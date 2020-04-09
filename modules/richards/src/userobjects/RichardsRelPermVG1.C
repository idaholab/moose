//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  "VG1" form of relative permeability
//
#include "RichardsRelPermVG1.h"

registerMooseObject("RichardsApp", RichardsRelPermVG1);

InputParameters
RichardsRelPermVG1::validParams()
{
  InputParameters params = RichardsRelPermVG::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "simm",
      "simm >=0 & simm < 1",
      "Immobile saturation.  Must be between 0 and 1.  Define s = "
      "(seff - simm)/(1 - simm).  Then relperm = s^(1/2) * (1 - (1 "
      "- s^(1/m))^m)^2");
  params.addRequiredRangeCheckedParam<Real>(
      "m",
      "m > 0 & m < 1",
      "van-Genuchten m parameter.  Must be between 0 and 1, and optimally "
      "should be set >0.5.  Define s = (seff - simm)/(1 - simm).  Then "
      "relperm = s^(1/2) * (1 - (1 - s^(1/m))^m)^2");
  params.addRequiredRangeCheckedParam<Real>(
      "scut", "scut > 0 & scut < 1", "cutoff in effective saturation.");
  params.addClassDescription("VG1 form of relative permeability.  Define s = (seff - simm)/(1 - "
                             "simm).  Then relperm = s^(1/2) * (1 - (1 - s^(1/m))^m)^2, if s>0, "
                             "and relperm=0 otherwise");
  return params;
}

RichardsRelPermVG1::RichardsRelPermVG1(const InputParameters & parameters)
  : RichardsRelPermVG(parameters),
    _simm(getParam<Real>("simm")),
    _m(getParam<Real>("m")),
    _scut(getParam<Real>("scut")),
    _vg1_const(0),
    _vg1_linear(0),
    _vg1_quad(0),
    _vg1_cub(0)
{
  _vg1_const = RichardsRelPermVG::relperm(_scut);
  _vg1_linear = RichardsRelPermVG::drelperm(_scut);
  _vg1_quad = RichardsRelPermVG::d2relperm(_scut);
  _vg1_cub = (1 - _vg1_const - _vg1_linear * (1 - _scut) - _vg1_quad * std::pow(1 - _scut, 2)) /
             std::pow(1 - _scut, 3);
}

void
RichardsRelPermVG1::initialSetup()
{
  _console << "Relative permeability of VG1 type has cubic coefficients " << _vg1_const << " "
           << _vg1_linear << " " << _vg1_quad << " " << _vg1_cub << std::endl;
}

Real
RichardsRelPermVG1::relperm(Real seff) const
{
  if (seff >= 1.0)
    return 1.0;

  if (seff <= _simm)
    return 0.0;

  Real s_internal = (seff - _simm) / (1.0 - _simm);

  if (s_internal < _scut)
    return RichardsRelPermVG::relperm(seff);

  Real krel = _vg1_const + _vg1_linear * (s_internal - _scut) +
              _vg1_quad * std::pow(s_internal - _scut, 2) +
              _vg1_cub * std::pow(s_internal - _scut, 3);

  // bound, just in case
  if (krel < 0)
  {
    krel = 0;
  }
  if (krel > 1)
  {
    krel = 1;
  }
  return krel;
}

Real
RichardsRelPermVG1::drelperm(Real seff) const
{
  if (seff >= 1.0)
    return 0.0;

  if (seff <= _simm)
    return 0.0;

  Real s_internal = (seff - _simm) / (1.0 - _simm);

  if (s_internal < _scut)
    return RichardsRelPermVG::drelperm(seff);

  Real krelp = _vg1_linear + 2 * _vg1_quad * (s_internal - _scut) +
               3 * _vg1_cub * std::pow(s_internal - _scut, 2);
  return krelp / (1.0 - _simm);
}

Real
RichardsRelPermVG1::d2relperm(Real seff) const
{
  if (seff >= 1.0)
    return 0.0;

  if (seff <= _simm)
    return 0.0;

  Real s_internal = (seff - _simm) / (1.0 - _simm);

  if (s_internal < _scut)
    return RichardsRelPermVG::d2relperm(seff);

  Real krelpp = 2 * _vg1_quad + 6 * _vg1_cub * (s_internal - _scut);
  return krelpp / std::pow(1.0 - _simm, 2);
}
