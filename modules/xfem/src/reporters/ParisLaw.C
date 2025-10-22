//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParisLaw.h"
#include "VectorPostprocessorInterface.h"
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
  params.addParam<VectorPostprocessorName>(
      "kii_vectorpostprocessor", "II_KII_1", "Name of the vectorpostprocessor that computes K_II");
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
    _kii_vpp(getVectorPostprocessorValue(
        "kii_vectorpostprocessor", getParam<VectorPostprocessorName>("kii_vectorpostprocessor"))),
    _dn(declareValueByName<Real>(getParam<ReporterValueName>("cycles_to_max_growth_increment_name"),
                                 REPORTER_MODE_ROOT)),
    _growth_increment(declareValueByName<std::vector<Real>>(
        getParam<ReporterValueName>("growth_increment_name"), REPORTER_MODE_ROOT))
{
}

void
ParisLaw::computeGrowth(std::vector<int> & index)
{
  _growth_increment.resize(_ki_x.size(), 0.0);
  std::vector<Real> effective_k(_ki_x.size(), 0.0);
  for (std::size_t i = 0; i < _ki_vpp.size(); ++i)
    if (index[i] != -1)
      effective_k[i] = std::sqrt(Utility::pow<2>(_ki_vpp[i]) + 2 * Utility::pow<2>(_kii_vpp[i]));

  Real _max_k = *std::max_element(effective_k.begin(), effective_k.end());
  if (_max_k == 0)
    _dn = std::numeric_limits<Real>::max();
  else
    _dn = _max_growth_increment / (_paris_law_c * std::pow(_max_k, _paris_law_m));

  for (std::size_t i = 0; i < _ki_vpp.size(); ++i)
    if (index[i] != -1)
    {
      if (_max_k == 0)
        _growth_increment[i] = 0;
      else
        _growth_increment[i] =
            _max_growth_increment * std::pow(effective_k[i] / _max_k, _paris_law_m);
    }
}
