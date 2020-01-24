//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StatisticsVectorPostprocessor.h"

// MOOSE includes
#include "MooseVariable.h"
#include "ThreadedElementLoopBase.h"
#include "ThreadedNodeLoop.h"

#include "libmesh/quadrature.h"

#include <numeric>

registerMooseObject("MooseApp", StatisticsVectorPostprocessor);

defineLegacyParams(StatisticsVectorPostprocessor);

InputParameters
StatisticsVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "Compute statistical values of a given VectorPostprocessor objects and vectors.");

  // TODO: Make these Required when deprecated are removed
  params.addParam<std::vector<VectorPostprocessorName>>(
      "vectorpostprocessors",
      "List of VectorPostprocessor(s) to utilized for statistic computations.");

  MultiMooseEnum stats = Statistics::makeCalculatorEnum();
  params.addParam<MultiMooseEnum>(
      "compute",
      stats,
      "The statistics to compute for each of the supplied vector postprocessors.");

  // DEPRECATED
  params.addDeprecatedParam<VectorPostprocessorName>(
      "vpp",
      "The VectorPostprocessor to compute statistics for.",
      "Replaced by 'vectorpostprocessors'");
  params.addDeprecatedParam<MultiMooseEnum>(
      "stats",
      MultiMooseEnum("min=0 max=1 sum=2 average=3 stddev=4 norm2=5 ratio=6"),
      "The statistics you would like to compute for each column of the VectorPostprocessor",
      "Replaced with 'compute'");
  return params;
}

StatisticsVectorPostprocessor::StatisticsVectorPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _compute_stats(isParamValid("stats") ? getParam<MultiMooseEnum>("stats")
                                         : getParam<MultiMooseEnum>("compute")),
    _stat_type_vector(declareVector("stat_type"))
{
  for (const auto & item : _compute_stats)
    _stat_type_vector.push_back(item.id());
}

void
StatisticsVectorPostprocessor::initialSetup()
{

  // DEPRECATED
  if (isParamValid("vpp"))
  {
    const auto & vpp_name = getParam<VectorPostprocessorName>("vpp");
    const std::vector<std::pair<std::string, VectorPostprocessorData::VectorPostprocessorState>> &
        vpp_vectors = _fe_problem.getVectorPostprocessorVectors(vpp_name);
    for (const auto & the_pair : vpp_vectors)
    {
      _stat_calculators.emplace_back(std::vector<std::unique_ptr<Statistics::Calculator>>());
      for (const auto & item : _compute_stats)
        _stat_calculators.back().emplace_back(Statistics::makeCalculator(item.name(), *this));
      _compute_from_names.emplace_back(vpp_name, the_pair.first, the_pair.second.is_distributed);
      _stat_vectors.push_back(&declareVector(the_pair.first));
    }
  }
  else
  {
    const auto & vpp_names = getParam<std::vector<VectorPostprocessorName>>("vectorpostprocessors");
    for (const auto & vpp_name : vpp_names)
    {
      const std::vector<std::pair<std::string, VectorPostprocessorData::VectorPostprocessorState>> &
          vpp_vectors = _fe_problem.getVectorPostprocessorVectors(vpp_name);
      for (const auto & the_pair : vpp_vectors)
      {
        // Create Statistics::Calculator objects for each statistic to be computed on each vector
        _stat_calculators.emplace_back(std::vector<std::unique_ptr<Statistics::Calculator>>());
        for (const auto & item : _compute_stats)
          _stat_calculators.back().emplace_back(Statistics::makeCalculator(item.name(), *this));

        // Store VectorPostprocessor name and vector name from which stats will be computed
        _compute_from_names.emplace_back(vpp_name, the_pair.first, the_pair.second.is_distributed);

        // Create the vector where the statistics will be stored
        std::string name = vpp_name + "_" + the_pair.first;
        _stat_vectors.push_back(&declareVector(name));
      }
    }
  }
}

void
StatisticsVectorPostprocessor::initialize()
{
  for (std::size_t i = 0; i < _compute_from_names.size(); ++i)
  {
    const bool is_distributed = std::get<2>(_compute_from_names[i]);
    if (is_distributed || processor_id() == 0)
    {
      for (auto & calc_ptr : _stat_calculators[i])
        calc_ptr->initialize(is_distributed);
    }
  }
}

void
StatisticsVectorPostprocessor::execute()
{
  for (std::size_t i = 0; i < _compute_from_names.size(); ++i)
  {
    const std::string & vpp_name = std::get<0>(_compute_from_names[i]);
    const std::string & vec_name = std::get<1>(_compute_from_names[i]);
    const bool is_distributed = std::get<2>(_compute_from_names[i]);
    const VectorPostprocessorValue & data =
        _fe_problem.getVectorPostprocessorValue(vpp_name, vec_name, true);

    if (is_distributed || processor_id() == 0)
    {
      for (auto & calc_ptr : _stat_calculators[i])
        calc_ptr->execute(data, is_distributed);
    }
  }
}

void
StatisticsVectorPostprocessor::finalize()
{
  for (std::size_t i = 0; i < _compute_from_names.size(); ++i)
  {
    const bool is_distributed = std::get<2>(_compute_from_names[i]);
    for (auto & calc_ptr : _stat_calculators[i])
    {
      if (is_distributed || processor_id() == 0)
        calc_ptr->finalize(is_distributed);
      _stat_vectors[i]->emplace_back(calc_ptr->value());
    }
  }
}
