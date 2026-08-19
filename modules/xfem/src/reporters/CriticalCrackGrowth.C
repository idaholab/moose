//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CriticalCrackGrowth.h"

registerMooseObject("XFEMApp", CriticalCrackGrowth);

InputParameters
CriticalCrackGrowth::validParams()
{
  InputParameters params = CrackGrowthReporterBase::validParams();
  params.addClassDescription(
      "Computes crack growth increments at active crack front points using a critical mixed-mode "
      "stress intensity factor criterion.");
  params.addRequiredRangeCheckedParam<Real>(
      "k_critical", "k_critical>0", "Critical fracture toughness.");
  params.addParam<ReporterValueName>(
      "growth_increment_name",
      "growth_increment",
      "Reporter value name for the crack growth increments at the crack front points.");
  return params;
}

CriticalCrackGrowth::CriticalCrackGrowth(const InputParameters & parameters)
  : CrackGrowthReporterBase(parameters),
    _k_critical(getParam<Real>("k_critical")),
    _growth_increment(declareValueByName<std::vector<Real>>(
        getParam<ReporterValueName>("growth_increment_name"), REPORTER_MODE_ROOT))
{
}

void
CriticalCrackGrowth::computeGrowth(std::vector<int> & index)
{
  _growth_increment.assign(_ki_vpp.size(), 0.0);

  // index is sized by the active-boundary node count, which can be one or two nodes shorter than
  // _ki_vpp (_crack_front_points) when an active tip grows outside the body and flips to an
  // inactive endpoint. Bound the loop by index; the trailing _growth_increment entries stay 0.0.
  for (const auto i : index_range(index))
  {
    if (index[i] == -1 || _ki_vpp[i] <= 0.0)
      continue;

    const Real effective_k_squared = _ki_vpp[i] * _ki_vpp[i] + _kii_vpp[i] * _kii_vpp[i];
    if (effective_k_squared > _k_critical * _k_critical)
      _growth_increment[i] = _max_growth_increment;
  }
}
