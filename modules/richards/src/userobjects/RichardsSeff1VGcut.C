//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  "cut" van-Genuchten effective saturation as a function of pressure, and its derivs wrt p
//
#include "RichardsSeff1VGcut.h"

registerMooseObject("RichardsApp", RichardsSeff1VGcut);

InputParameters
RichardsSeff1VGcut::validParams()
{
  InputParameters params = RichardsSeff1VG::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "p_cut",
      "p_cut < 0",
      "cutoff in pressure.  Must be negative.  If p>p_cut then use "
      "van-Genuchten function.  Otherwise use a linear relationship which is "
      "chosen so the value and derivative match van-Genuchten at p=p_cut");
  params.addClassDescription("cut van-Genuchten effective saturation as a function of capillary "
                             "pressure.  Single-phase  seff = (1 + (-al*p)^(1/(1-m)))^(-m) for "
                             "p>p_cut, otherwise user a a linear relationship that is chosen so "
                             "the value and derivative match van-Genuchten at p=p_cut.");
  return params;
}

RichardsSeff1VGcut::RichardsSeff1VGcut(const InputParameters & parameters)
  : RichardsSeff1VG(parameters),
    _al(getParam<Real>("al")),
    _m(getParam<Real>("m")),
    _p_cut(getParam<Real>("p_cut")),
    _s_cut(0),
    _ds_cut(0)
{
  _s_cut = RichardsSeffVG::seff(_p_cut, _al, _m);
  _ds_cut = RichardsSeffVG::dseff(_p_cut, _al, _m);
}

void
RichardsSeff1VGcut::initialSetup()
{
  _console << "cut VG Seff has p_cut=" << _p_cut << " so seff_cut=" << _s_cut
           << " and seff=0 at p=" << -_s_cut / _ds_cut + _p_cut << std::endl;
}

Real
RichardsSeff1VGcut::seff(std::vector<const VariableValue *> p, unsigned int qp) const
{
  if ((*p[0])[qp] > _p_cut)
  {
    return RichardsSeff1VG::seff(p, qp);
  }
  else
  {
    Real seff_linear = _s_cut + _ds_cut * ((*p[0])[qp] - _p_cut);
    // return (seff_linear > 0 ? seff_linear : 0); // andy isn't sure of this - might be useful to
    // allow negative saturations
    return seff_linear;
  }
}

void
RichardsSeff1VGcut::dseff(std::vector<const VariableValue *> p,
                          unsigned int qp,
                          std::vector<Real> & result) const
{
  if ((*p[0])[qp] > _p_cut)
    return RichardsSeff1VG::dseff(p, qp, result);
  else
    result[0] = _ds_cut;
}

void
RichardsSeff1VGcut::d2seff(std::vector<const VariableValue *> p,
                           unsigned int qp,
                           std::vector<std::vector<Real>> & result) const
{
  if ((*p[0])[qp] > _p_cut)
    return RichardsSeff1VG::d2seff(p, qp, result);
  else
    result[0][0] = 0;
}
