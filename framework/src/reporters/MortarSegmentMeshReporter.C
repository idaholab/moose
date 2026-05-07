//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarSegmentMeshReporter.h"
#include "AutomaticMortarGeneration.h"
#include "FEProblemBase.h"

registerMooseObject("MooseApp", MortarSegmentMeshReporter);

InputParameters
MortarSegmentMeshReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "Reports mortar segment mesh statistics (element counts and volume statistics) for all "
      "mortar interfaces. One entry per primary-secondary subdomain pair is appended to each "
      "output vector.");
  params.addRequiredParam<bool>(
      "on_displaced", "Whether to report statistics for the displaced mortar interfaces.");
  return params;
}

MortarSegmentMeshReporter::MortarSegmentMeshReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _on_displaced(getParam<bool>("on_displaced")),
    _secondary_lower_n_elems(declareValueByName<std::vector<unsigned int>>(
        "secondary_lower_n_elems", REPORTER_MODE_ROOT)),
    _secondary_lower_max_volume(
        declareValueByName<std::vector<Real>>("secondary_lower_max_volume", REPORTER_MODE_ROOT)),
    _secondary_lower_min_volume(
        declareValueByName<std::vector<Real>>("secondary_lower_min_volume", REPORTER_MODE_ROOT)),
    _secondary_lower_median_volume(
        declareValueByName<std::vector<Real>>("secondary_lower_median_volume", REPORTER_MODE_ROOT)),
    _primary_lower_n_elems(
        declareValueByName<std::vector<unsigned int>>("primary_lower_n_elems", REPORTER_MODE_ROOT)),
    _primary_lower_max_volume(
        declareValueByName<std::vector<Real>>("primary_lower_max_volume", REPORTER_MODE_ROOT)),
    _primary_lower_min_volume(
        declareValueByName<std::vector<Real>>("primary_lower_min_volume", REPORTER_MODE_ROOT)),
    _primary_lower_median_volume(
        declareValueByName<std::vector<Real>>("primary_lower_median_volume", REPORTER_MODE_ROOT)),
    _msm_n_elems(declareValueByName<std::vector<unsigned int>>("msm_n_elems", REPORTER_MODE_ROOT)),
    _msm_max_volume(declareValueByName<std::vector<Real>>("msm_max_volume", REPORTER_MODE_ROOT)),
    _msm_min_volume(declareValueByName<std::vector<Real>>("msm_min_volume", REPORTER_MODE_ROOT)),
    _msm_median_volume(
        declareValueByName<std::vector<Real>>("msm_median_volume", REPORTER_MODE_ROOT))
{
}

void
MortarSegmentMeshReporter::execute()
{
  _secondary_lower_n_elems.clear();
  _secondary_lower_max_volume.clear();
  _secondary_lower_min_volume.clear();
  _secondary_lower_median_volume.clear();
  _primary_lower_n_elems.clear();
  _primary_lower_max_volume.clear();
  _primary_lower_min_volume.clear();
  _primary_lower_median_volume.clear();
  _msm_n_elems.clear();
  _msm_max_volume.clear();
  _msm_min_volume.clear();
  _msm_median_volume.clear();

  for (const auto & [key, amg_ptr] : _fe_problem.getMortarInterfaces(_on_displaced))
  {
    const auto stats = amg_ptr->computeMsmStatistics();

    for (const auto & s : stats)
    {
      _secondary_lower_n_elems.push_back(s.secondary_lower_n_elems);
      _secondary_lower_max_volume.push_back(s.secondary_lower_max_volume);
      _secondary_lower_min_volume.push_back(s.secondary_lower_min_volume);
      _secondary_lower_median_volume.push_back(s.secondary_lower_median_volume);
      _primary_lower_n_elems.push_back(s.primary_lower_n_elems);
      _primary_lower_max_volume.push_back(s.primary_lower_max_volume);
      _primary_lower_min_volume.push_back(s.primary_lower_min_volume);
      _primary_lower_median_volume.push_back(s.primary_lower_median_volume);
      _msm_n_elems.push_back(s.msm_n_elems);
      _msm_max_volume.push_back(s.msm_max_volume);
      _msm_min_volume.push_back(s.msm_min_volume);
      _msm_median_volume.push_back(s.msm_median_volume);
    }
  }
}
