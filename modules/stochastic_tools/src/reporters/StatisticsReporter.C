//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StatisticsReporter.h"

// MOOSE includes
#include "MooseVariable.h"
#include "ThreadedElementLoopBase.h"
#include "ThreadedNodeLoop.h"

#include "libmesh/quadrature.h"

#include <numeric>

registerMooseObject("StochasticToolsApp", StatisticsReporter);

InputParameters
StatisticsReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "Compute statistical values of a given VectorPostprocessor objects and vectors.");

  params.addParam<std::vector<VectorPostprocessorName>>(
      "vectorpostprocessors",
      "List of VectorPostprocessor(s) to utilized for statistic computations.");

  params.addParam<std::vector<ReporterName>>(
      "reporters", "List of Reporter values to utilized for statistic computations.");

  MultiMooseEnum stats = StochasticTools::makeCalculatorEnum();
  params.addRequiredParam<MultiMooseEnum>(
      "compute",
      stats,
      "The statistic(s) to compute for each of the supplied vector postprocessors.");

  // Confidence Levels
  MooseEnum ci = StochasticTools::makeBootstrapCalculatorEnum();
  params.addParam<MooseEnum>(
      "ci_method", ci, "The method to use for computing confidence level intervals.");

  params.addParam<std::vector<Real>>(
      "ci_levels",
      std::vector<Real>({0.1, 0.9}),
      "A vector of confidence levels to consider, values must be in (0, 1).");
  params.addParam<unsigned int>(
      "ci_replicates",
      10000,
      "The number of replicates to use when computing confidence level intervals.");
  params.addParam<unsigned int>("ci_seed",
                                1,
                                "The random number generator seed used for creating replicates "
                                "while computing confidence level intervals.");
  return params;
}

StatisticsReporter::StatisticsReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _compute_stats(getParam<MultiMooseEnum>("compute")),
    _ci_method(getParam<MooseEnum>("ci_method")),
    _ci_levels(getParam<std::vector<Real>>("ci_levels")),
    _ci_replicates(getParam<unsigned int>("ci_replicates")),
    _ci_seed(getParam<unsigned int>("ci_seed")),
    _initialized(false)
{
  // CI levels error checking
  if (_ci_method.isValid())
  {
    if (_ci_levels.empty())
      paramError("ci_levels",
                 "If the 'ci_method' parameter is supplied then the 'ci_levels' must also be "
                 "supplied with values in (0, 1).");

    else if (*std::min_element(_ci_levels.begin(), _ci_levels.end()) <= 0)
      paramError("ci_levels", "The supplied levels must be greater than zero.");

    else if (*std::max_element(_ci_levels.begin(), _ci_levels.end()) >= 1)
      paramError("ci_levels", "The supplied levels must be less than 1.0");
  }

  if ((!isParamValid("reporters") && !isParamValid("vectorpostprocessors")) ||
      (getParam<std::vector<ReporterName>>("reporters").empty() &&
       getParam<std::vector<VectorPostprocessorName>>("vectorpostprocessors").empty()))
    mooseError(
        "The 'vectorpostprocessors' and/or 'reporters' parameters must be defined and non-empty.");
}

void
StatisticsReporter::initialize()
{
  if (_initialized)
    return;

  // Stats for Reporters
  if (isParamValid("reporters"))
  {
    std::vector<std::string> unsupported_types;
    const auto & reporter_names = getParam<std::vector<ReporterName>>("reporters");
    for (const auto & r_name : reporter_names)
    {
      if (hasReporterValueByName<std::vector<Real>>(r_name))
        declareValueHelper<std::vector<Real>, Real>(r_name);
      else if (hasReporterValueByName<std::vector<int>>(r_name))
        declareValueHelper<std::vector<int>, Real>(r_name);
      else if (hasReporterValueByName<std::vector<std::vector<Real>>>(r_name))
        declareValueHelper<std::vector<std::vector<Real>>, std::vector<Real>>(r_name);
      else
        unsupported_types.emplace_back(r_name.getCombinedName());
    }

    if (!unsupported_types.empty())
      paramError("reporters",
                 "The following reporter value(s) do not have a type supported by the "
                 "StatisticsReporter:\n",
                 MooseUtils::join(unsupported_types, ", "));
  }

  // Stats for VPP
  if (isParamValid("vectorpostprocessors"))
  {
    const auto & vpp_names = getParam<std::vector<VectorPostprocessorName>>("vectorpostprocessors");
    for (const auto & vpp_name : vpp_names)
    {
      const VectorPostprocessor & vpp_object =
          _fe_problem.getVectorPostprocessorObjectByName(vpp_name);
      const std::set<std::string> & vpp_vectors = vpp_object.getVectorNames();
      for (const auto & vec_name : vpp_vectors)
      {
        ReporterName r_name(vpp_name, vec_name);
        declareValueHelper<std::vector<Real>, Real>(r_name);
      }
    }
  }

  _initialized = true;
}

void
StatisticsReporter::store(nlohmann::json & json) const
{
  Reporter::store(json);
  if (_ci_method.isValid())
    json["confidence_intervals"] = {{"method", _ci_method},
                                    {"levels", _ci_levels},
                                    {"replicates", _ci_replicates},
                                    {"seed", _ci_seed}};
}

template <typename InType, typename OutType>
void
StatisticsReporter::declareValueHelper(const ReporterName & r_name)
{
  const auto & mode = _fe_problem.getReporterData().getReporterMode(r_name);
  const auto & data = getReporterValueByName<InType>(r_name);
  for (const auto & item : _compute_stats)
  {
    const std::string s_name =
        r_name.getObjectName() + "_" + r_name.getValueName() + "_" + item.name();
    if (_ci_method.isValid())
      declareValueByName<std::pair<OutType, std::vector<OutType>>,
                         ReporterStatisticsContext<InType, OutType>>(s_name,
                                                                     REPORTER_MODE_ROOT,
                                                                     data,
                                                                     mode,
                                                                     item,
                                                                     _ci_method,
                                                                     _ci_levels,
                                                                     _ci_replicates,
                                                                     _ci_seed);
    else
      declareValueByName<std::pair<OutType, std::vector<OutType>>,
                         ReporterStatisticsContext<InType, OutType>>(
          s_name, REPORTER_MODE_ROOT, data, mode, item);
  }
}

template void
StatisticsReporter::declareValueHelper<std::vector<Real>, Real>(const ReporterName & r_name);
template void
StatisticsReporter::declareValueHelper<std::vector<int>, Real>(const ReporterName & r_name);
