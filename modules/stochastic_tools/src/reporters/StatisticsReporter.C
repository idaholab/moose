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
      "ci_levels", "A vector of confidence levels to consider, values must be in (0, 0.5].");
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
  : GeneralReporter(parameters)
{
  // Statistics to be computed
  const auto & compute_stats = getParam<MultiMooseEnum>("compute");

  // Bootstrap CI
  std::unique_ptr<const StochasticTools::BootstrapCalculator> ci_calculator = nullptr;
  const MooseEnum & ci_method = getParam<MooseEnum>("ci_method");
  if (ci_method.isValid())
  {
    std::vector<Real> ci_levels = computeLevels(getParam<std::vector<Real>>("ci_levels"));
    unsigned int replicates = getParam<unsigned int>("ci_replicates");
    unsigned int seed = getParam<unsigned int>("ci_seed");
    _ci_calculator =
        StochasticTools::makeBootstrapCalculator(ci_method, *this, ci_levels, replicates, seed);
  }

  // Stats for Reporters
  if (isParamValid("reporters"))
  {
    const auto & reporter_names = getParam<std::vector<ReporterName>>("reporters");
    for (const auto & r_name : reporter_names)
    {
      const auto & data = getReporterValueByName<std::vector<Real>>(r_name);
      const auto & mode = _fe_problem.getReporterData().getReporterMode(r_name);

      for (const auto & item : compute_stats)
      {

        const std::string s_name = r_name.getCombinedName() + "_" + item.name();
        if (hasReporterValueByName<std::vector<Real>>(r_name))
          declareValueByName<Real, ReporterStatisticsContext>(
              s_name, REPORTER_MODE_ROOT, data, mode, item, _ci_calculator.get());
        else
          paramError("reporters",
                     "The reporter value '",
                     r_name,
                     "' is not a type supported by the StatisticsReporter.");
      }
    }
  }

  // Stats for VPP
  else if (isParamValid("vectorpostprocessors"))
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
        const auto & data = getReporterValueByName<std::vector<Real>>(r_name);
        const auto & mode = _fe_problem.getReporterData().getReporterMode(r_name);
        for (const auto & item : compute_stats)
        {
          const std::string s_name = vpp_name + "::" + vec_name + "_" + item.name();
          declareValueByName<Real, ReporterStatisticsContext>(
              s_name, REPORTER_MODE_ROOT, data, mode, item, _ci_calculator.get());
        }
      }
    }
  }

  else
    mooseError("The 'vectorpostprocessors' and/or 'reporters' parameters must be defined.");
}

std::vector<Real>
StatisticsReporter::computeLevels(const std::vector<Real> & levels_in) const
{
  if (levels_in.empty())
    paramError("ci_levels",
               "If the 'ci_method' parameter is supplied then the 'ci_levels' must also be "
               "supplied with values in (0, 0.5].");

  else if (*std::min_element(levels_in.begin(), levels_in.end()) <= 0)
    paramError("ci_levels", "The supplied levels must be greater than zero.");

  else if (*std::max_element(levels_in.begin(), levels_in.end()) > 0.5)
    paramError("ci_levels", "The supplied levels must be less than or equal to 0.5");

  std::list<Real> levels_out;
  for (auto it = levels_in.rbegin(); it != levels_in.rend(); ++it)
  {
    if (*it == 0.5)
      levels_out.push_back(*it);

    else
    {
      levels_out.push_front(*it);
      levels_out.push_back(1 - *it);
    }
  }
  return std::vector<Real>(levels_out.begin(), levels_out.end());
}
