//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParisLaw.h"
#include "MathUtils.h"
#include <limits>

registerMooseObject("XFEMApp", ParisLaw);

InputParameters
ParisLaw::validParams()
{
  InputParameters params = CrackGrowthReporterBase::validParams();
  params.addClassDescription(
      "This reporter computes the crack growth increment at all active crack front points "
      "in the CrackMeshCut3DUserObject for fatigue crack growth based on the Paris Law. "
      "Data for crack growth rates in this reporter are stored in the same order as in the "
      "fracture integral VectorPostprocessors.");
  params.addRequiredParam<Real>("paris_law_c", "parameter C in the Paris law for fatigue");
  params.addRequiredParam<Real>("paris_law_m", "parameter m in the Paris law for fatigue");
  params.addParam<ReporterValueName>(
      "growth_increment_name",
      "growth_increment",
      "ReporterValueName for storing computed growth increments for the crack front points.");
  params.addParam<ReporterValueName>(
      "cycles_to_max_growth_increment_name",
      "dN",
      "ReporterValueName for storing computed number of cycles to reach max_growth_increment.");
  return params;
}

ParisLaw::ParisLaw(const InputParameters & parameters)
  : CrackGrowthReporterBase(parameters),
    _paris_law_c(getParam<Real>("paris_law_c")),
    _paris_law_m(getParam<Real>("paris_law_m")),
    _dn(declareValueByName<Real>(getParam<ReporterValueName>("cycles_to_max_growth_increment_name"),
                                 REPORTER_MODE_ROOT)),
    _growth_increment(declareValueByName<std::vector<Real>>(
        getParam<ReporterValueName>("growth_increment_name"), REPORTER_MODE_ROOT))
{
}

void
ParisLaw::computeGrowth(const std::vector<int> & index)
{
  _growth_increment.assign(_ki_vpp.size(), 0.0);

  // The fracture integrals have not been computed yet on the first execution, so there is nothing
  // to evaluate.
  if (_ki_vpp.empty())
    return;

  // index is keyed by active-boundary position, but its value is the position of that node in
  // _crack_front_points, which is the ordering of the fracture-integral VectorPostprocessors and of
  // _growth_increment.  index is also sized by the active-boundary node count, which can be one or
  // two nodes shorter than _ki_vpp when an active tip grows outside the body and flips to an
  // inactive endpoint.  So loop over index and address everything else through it.  Inactive
  // endpoints carry -1 and keep their 0.0 increment, as do any crack front points no longer active.
  std::vector<Real> effective_k(_ki_vpp.size(), 0.0);
  for (const auto i : index_range(index))
  {
    const int k = index[i];
    if (k == -1)
      continue;
    effective_k.at(k) =
        std::sqrt(Utility::pow<2>(_ki_vpp.at(k)) + 2 * Utility::pow<2>(_kii_vpp.at(k)));
  }

  const Real max_k = *std::max_element(effective_k.begin(), effective_k.end());
  if (max_k == 0)
  {
    _dn = std::numeric_limits<Real>::max();
    // Every increment stays at the 0.0 assigned above.
    return;
  }

  _dn = _max_growth_increment / (_paris_law_c * std::pow(max_k, _paris_law_m));

  for (const auto i : index_range(index))
  {
    const int k = index[i];
    if (k == -1)
      continue;
    _growth_increment.at(k) =
        _max_growth_increment * std::pow(effective_k.at(k) / max_k, _paris_law_m);
  }
}
