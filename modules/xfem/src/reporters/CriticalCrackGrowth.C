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
CriticalCrackGrowth::computeGrowth(const std::vector<int> & index)
{
  _growth_increment.assign(_ki_vpp.size(), 0.0);

  // The fracture integrals have not been computed yet on the first execution, so there is nothing
  // to evaluate.
  if (_ki_vpp.empty())
    return;

  // index is keyed by active-boundary position, but its value is the position of that node in
  // _crack_front_points, which is the ordering of the fracture-integral VectorPostprocessors and of
  // _growth_increment.  The two orderings are not the same (_crack_front_points is stored in the
  // reverse order of the active boundary), and index is sized by the active-boundary node count,
  // which can be one or two nodes shorter than _ki_vpp when an active tip grows outside the body
  // and flips to an inactive endpoint.  So loop over index and address everything else through it.
  // Inactive endpoints carry -1 and keep their 0.0 increment, as do any trailing entries.
  for (const auto i : index_range(index))
  {
    const int k = index[i];
    if (k == -1 || _ki_vpp.at(k) <= 0.0)
      continue;

    const Real effective_k_squared =
        _ki_vpp.at(k) * _ki_vpp.at(k) + _kii_vpp.at(k) * _kii_vpp.at(k);
    if (effective_k_squared > _k_critical * _k_critical)
      _growth_increment.at(k) = _max_growth_increment;
  }
}
