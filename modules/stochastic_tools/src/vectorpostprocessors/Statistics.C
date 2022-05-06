//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Statistics.h"

// MOOSE includes
#include "MooseVariable.h"
#include "ThreadedElementLoopBase.h"
#include "ThreadedNodeLoop.h"

#include "libmesh/quadrature.h"

#include <numeric>

registerADMooseObjectDeprecated("StochasticToolsApp", Statistics, "07/01/2021 12:00");

InputParameters
Statistics::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "Compute statistical values of a given VectorPostprocessor objects and vectors.");

  params.addRequiredParam<std::vector<VectorPostprocessorName>>(
      "vectorpostprocessors",
      "List of VectorPostprocessor(s) to utilized for statistic computations.");

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

  // Compute values are computed on rank 0 and broadcast
  params.set<MooseEnum>("parallel_type") = "REPLICATED";
  params.suppressParameter<MooseEnum>("parallel_type");
  params.set<bool>("_auto_boradcast") = true;

  return params;
}

Statistics::Statistics(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _compute_stats(getParam<MultiMooseEnum>("compute")),
    _ci_method(getParam<MooseEnum>("ci_method")),
    _ci_levels(_ci_method.isValid() ? computeLevels(getParam<std::vector<Real>>("ci_levels"))
                                    : std::vector<Real>()),
    _replicates(getParam<unsigned int>("ci_replicates")),
    _seed(getParam<unsigned int>("ci_seed")),
    _stat_type_vector(declareVector("stat_type"))
{
  for (const auto & item : _compute_stats)
  {
    _stat_type_vector.push_back(item.id());
    for (const auto & level : _ci_levels)
      _stat_type_vector.push_back(item.id() + level);
  }
}

void
Statistics::initialSetup()
{
  TIME_SECTION("initialSetup", 3, "Setting Up Statistics");

  const auto & vpp_names = getParam<std::vector<VectorPostprocessorName>>("vectorpostprocessors");
  for (const auto & vpp_name : vpp_names)
  {
    const VectorPostprocessor & vpp_object =
        _fe_problem.getVectorPostprocessorObjectByName(vpp_name);
    const std::set<std::string> & vpp_vectors = vpp_object.getVectorNames();
    for (const auto & vec_name : vpp_vectors)
    {
      // Store VectorPostprocessor name and vector name from which stats will be computed
      _compute_from_names.emplace_back(vpp_name, vec_name, vpp_object.isDistributed());

      // Create the vector where the statistics will be stored
      std::string name = vpp_name + "_" + vec_name;
      _stat_vectors.push_back(&declareVector(name));
    }
  }
}

void
Statistics::initialize()
{
  if (!containsCompleteHistory())
    for (const auto & vec : _stat_vectors)
      vec->clear();
}

void
Statistics::execute()
{
  TIME_SECTION("execute", 3, "Executing Statistics");

  for (std::size_t i = 0; i < _compute_from_names.size(); ++i)
  {
    const std::string & vpp_name = std::get<0>(_compute_from_names[i]);
    const std::string & vec_name = std::get<1>(_compute_from_names[i]);
    const bool is_distributed = std::get<2>(_compute_from_names[i]);
    const VectorPostprocessorValue & data =
        getVectorPostprocessorValueByName(vpp_name, vec_name, true);

    if (is_distributed || processor_id() == 0)
    {
      for (const auto & item : _compute_stats)
      {
        std::unique_ptr<StochasticTools::Calculator<std::vector<Real>, Real>> calc_ptr =
            StochasticTools::makeCalculator(item, *this);
        _stat_vectors[i]->emplace_back(calc_ptr->compute(data, is_distributed));

        if (_ci_method.isValid())
        {
          auto ci_calc_ptr = StochasticTools::makeBootstrapCalculator<std::vector<Real>, Real>(
              _ci_method, *this, _ci_levels, _replicates, _seed, *calc_ptr);
          std::vector<Real> ci = ci_calc_ptr->compute(data, is_distributed);
          _stat_vectors[i]->insert(_stat_vectors[i]->end(), ci.begin(), ci.end());
        }
      }
    }
  }
}

std::vector<Real>
Statistics::computeLevels(const std::vector<Real> & levels_in) const
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
