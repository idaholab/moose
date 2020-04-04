//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowHalfCubicSink.h"
#include "libmesh/utility.h"

registerMooseObject("PorousFlowApp", PorousFlowHalfCubicSink);

InputParameters
PorousFlowHalfCubicSink::validParams()
{
  InputParameters params = PorousFlowSinkPTDefiner::validParams();
  params.addRequiredParam<Real>(
      "max",
      "Maximum of the cubic flux multiplier.  Denote x = porepressure - center (or in the "
      "case of a heat flux with no fluid, x = temperature - center).  Then Flux out is "
      "multiplied by (max/cutoff^3)*(2x + cutoff)(x - cutoff)^2 for cutoff < x < 0.  Flux "
      "out is multiplied by max for x >= 0.  Flux out is multiplied by 0 for x <= cutoff.");
  params.addRequiredParam<FunctionName>("cutoff",
                                        "Cutoff of the cubic (measured in Pa (or K for "
                                        "temperature BCs)).  This needs to be less than "
                                        "zero.");
  params.addRequiredParam<Real>(
      "center", "Center of the cubic flux multiplier (measured in Pa (or K for temperature BCs)).");
  params.addClassDescription("Applies a flux sink to a boundary. The base flux defined by "
                             "PorousFlowSink is multiplied by a cubic.");
  return params;
}

PorousFlowHalfCubicSink::PorousFlowHalfCubicSink(const InputParameters & parameters)
  : PorousFlowSinkPTDefiner(parameters),
    _maximum(getParam<Real>("max")),
    _cutoff(getFunction("cutoff")),
    _center(getParam<Real>("center"))
{
}

Real
PorousFlowHalfCubicSink::multiplier() const
{
  const Real x = ptVar() - _center;

  if (x >= 0)
    return PorousFlowSink::multiplier() * _maximum;

  const Real cutoff = _cutoff.value(_t, _q_point[_qp]);
  if (x <= cutoff)
    return 0.0;

  return PorousFlowSink::multiplier() * _maximum * (2 * x + cutoff) * (x - cutoff) * (x - cutoff) /
         Utility::pow<3>(cutoff);
}

Real
PorousFlowHalfCubicSink::dmultiplier_dvar(unsigned int pvar) const
{
  const Real x = ptVar() - _center;

  if (x >= 0)
    return PorousFlowSink::dmultiplier_dvar(pvar) * _maximum;

  const Real cutoff = _cutoff.value(_t, _q_point[_qp]);
  if (x <= cutoff)
    return 0.0;

  const Real str =
      _maximum * (2 * x + cutoff) * (x - cutoff) * (x - cutoff) / Utility::pow<3>(cutoff);
  const Real deriv = _maximum * 6 * x * (x - cutoff) / Utility::pow<3>(cutoff);
  return PorousFlowSink::dmultiplier_dvar(pvar) * str +
         PorousFlowSink::multiplier() * deriv * dptVar(pvar);
}
