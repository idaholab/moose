//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParisLawVpp.h"
#include "VectorPostprocessorInterface.h"
#include "MathUtils.h"

registerMooseObject("XFEMApp", ParisLawVpp);

InputParameters
ParisLawVpp::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Computes the crack extension size at all active crack front points.");
  params.addRequiredParam<Real>(
      "max_growth_size",
      "the max growth size at the crack front in each increment of a fatigue simulation");
  params.addRequiredParam<Real>("paris_law_c", "parameter C in the Paris law for fatigue");
  params.addRequiredParam<Real>("paris_law_m", "parameter m in the Paris law for fatigue");
  params.addParam<VectorPostprocessorName>(
      "ki_vectorpostprocessor", "II_KI_1", "The name of the vectorpostprocessor that contains KI");
  params.addParam<VectorPostprocessorName>("kii_vectorpostprocessor",
                                           "II_KII_1",
                                           "The name of the vectorpostprocessor that contains KII");
  return params;
}

ParisLawVpp::ParisLawVpp(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _max_growth_size(getParam<Real>("max_growth_size")),
    _paris_law_c(getParam<Real>("paris_law_c")),
    _paris_law_m(getParam<Real>("paris_law_m")),
    _ki_vpp(getVectorPostprocessorValue(
        "ki_vectorpostprocessor", getParam<VectorPostprocessorName>("ki_vectorpostprocessor"))),
    _kii_vpp(getVectorPostprocessorValue(
        "kii_vectorpostprocessor", getParam<VectorPostprocessorName>("kii_vectorpostprocessor"))),
    _ki_x(getVectorPostprocessorValue("ki_vectorpostprocessor","x")),
    _ki_y(getVectorPostprocessorValue("ki_vectorpostprocessor","y")),
    _ki_z(getVectorPostprocessorValue("ki_vectorpostprocessor","z")),
    _ki_id(getVectorPostprocessorValue("ki_vectorpostprocessor","id")),
    _dn(declareVector("cycles_to_max_growth_size")),
    _growth_increment(declareVector("paris_law_growth_increment")),
    _x(declareVector("x")),
    _y(declareVector("y")),
    _z(declareVector("z")),
    _id(declareVector("id"))
{
}

void
ParisLawVpp::execute()
{
  _x.clear();
  _y.clear();
  _z.clear();
  _id.clear();

  _x.resize(_ki_x.size());
  _y.resize(_ki_y.size());
  _z.resize(_ki_z.size());
  _id.resize(_ki_id.size());
  std::copy(_ki_x.begin(), _ki_x.end(), _x.begin());
  std::copy(_ki_y.begin(), _ki_y.end(), _y.begin());
  std::copy(_ki_z.begin(), _ki_z.end(), _z.begin());
  std::copy(_ki_id.begin(), _ki_id.end(), _id.begin());

  _growth_increment.clear();
  _dn.clear();
  _dn.reserve(_ki_x.size());
  _growth_increment.reserve(_ki_x.size());

  std::vector<Real> effective_k(_ki_x.size());
  for (std::size_t i = 0; i < _ki_vpp.size(); ++i)
  {
    effective_k[i] = std::sqrt(Utility::pow<2>(_ki_vpp[i]) + 2 * Utility::pow<2>(_kii_vpp[i]));
    _dn.push_back(_max_growth_size / (_paris_law_c * std::pow(effective_k[i], _paris_law_m)));
  }

  Real _max_k = *std::max_element(effective_k.begin(), effective_k.end());
  for (auto && eff_k: effective_k)
    _growth_increment.push_back(_max_growth_size * std::pow(eff_k / _max_k, _paris_law_m));
}
