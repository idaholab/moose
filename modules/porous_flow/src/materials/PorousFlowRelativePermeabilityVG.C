//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowRelativePermeabilityVG.h"
#include "libmesh/utility.h"

template <>
InputParameters
validParams<PorousFlowRelativePermeabilityVG>()
{
  InputParameters params = validParams<PorousFlowRelativePermeabilityBase>();
  params.addRequiredRangeCheckedParam<Real>(
      "m", "m > 0 & m < 1", "The van Genuchten exponent of the phase");
  params.addRangeCheckedParam<Real>("seff_turnover",
                                    1.0,
                                    "seff_turnover > 0 & seff_turnover <= 1",
                                    "The relative permeability will be a cubic for seff > "
                                    "seff_turnover.  The cubic is chosen so that its derivative "
                                    "and value matche the VG function at seff=seff_turnover");
  params.addParam<bool>("zero_derivative",
                        false,
                        "Employ a cubic for seff>seff_turnover that has zero derivative at seff=1");
  params.addClassDescription(
      "This Material calculates relative permeability of a phase using the van Genuchten model");
  return params;
}

PorousFlowRelativePermeabilityVG::PorousFlowRelativePermeabilityVG(
    const InputParameters & parameters)
  : PorousFlowRelativePermeabilityBase(parameters),
    _m(getParam<Real>("m")),
    _cut(getParam<Real>("seff_turnover")),
    _cub0(PorousFlowVanGenuchten::relativePermeability(_cut, _m)),
    _cub1(PorousFlowVanGenuchten::dRelativePermeability(_cut, _m)),
    _cub2(_cut < 1.0
              ? (getParam<bool>("zero_derivative")
                     ? 3.0 * (1.0 - _cub0 - _cub1 * (1.0 - _cut)) / Utility::pow<2>(1.0 - _cut) +
                           _cub1 / (1.0 - _cut)
                     : PorousFlowVanGenuchten::d2RelativePermeability(_cut, _m))
              : 0.0),
    _cub3(_cut < 1.0
              ? (getParam<bool>("zero_derivative")
                     ? -2.0 * (1.0 - _cub0 - _cub1 * (1.0 - _cut)) / Utility::pow<3>(1.0 - _cut) -
                           _cub1 / Utility::pow<2>(1.0 - _cut)
                     : (1.0 - _cub0 - _cub1 * (1.0 - _cut) - _cub2 * Utility::pow<2>(1.0 - _cut)) /
                           Utility::pow<3>(1.0 - _cut))
              : 0.0)
{
}

Real
PorousFlowRelativePermeabilityVG::relativePermeability(Real seff) const
{
  if (seff < _cut)
    return PorousFlowVanGenuchten::relativePermeability(seff, _m);

  return _cub0 + _cub1 * (seff - _cut) + _cub2 * Utility::pow<2>(seff - _cut) +
         _cub3 * Utility::pow<3>(seff - _cut);
}

Real
PorousFlowRelativePermeabilityVG::dRelativePermeability(Real seff) const
{
  if (seff < _cut)
    return PorousFlowVanGenuchten::dRelativePermeability(seff, _m);

  return _cub1 + 2.0 * _cub2 * (seff - _cut) + 3.0 * _cub3 * Utility::pow<2>(seff - _cut);
}
