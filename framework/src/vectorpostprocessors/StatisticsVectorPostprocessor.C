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

template <>
InputParameters
validParams<StatisticsVectorPostprocessor>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  params.addClassDescription("Compute statistical values of a given VectorPostprocessor.  The "
                             "statistics are computed for each column.");

  params.addRequiredParam<VectorPostprocessorName>(
      "vpp", "The VectorPostprocessor to compute statistics for.");

  // These are directly numbered to ensure that any changes to this list will not cause a bug
  // Do not change the numbers here without changing the corresponding code in computeStatVector()
  MultiMooseEnum stats("min=0 max=1 sum=2 average=3 stddev=4 norm2=5");
  params.addRequiredParam<MultiMooseEnum>(
      "stats",
      stats,
      "The statistics you would like to compute for each column of the VectorPostprocessor");

  return params;
}

StatisticsVectorPostprocessor::StatisticsVectorPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _vpp_name(getParam<VectorPostprocessorName>("vpp")),
    _stats(getParam<MultiMooseEnum>("stats")),
    _stat_type_vector(declareVector("stat_type"))
{
}

void
StatisticsVectorPostprocessor::initialize()
{
  const auto & vpp_vectors = _fe_problem.getVectorPostprocessorVectors(_vpp_name);

  // Add vectors for each column, the reason for the extra logic here is that the columns a VPP
  // produces can change
  for (const auto & the_pair : vpp_vectors)
  {
    const auto & name = the_pair.first;

    if (_stat_vectors.find(name) == _stat_vectors.end())
      _stat_vectors[name] = &declareVector(name);
  }
}

void
StatisticsVectorPostprocessor::execute()
{
  if (processor_id() == 0) // Only compute on processor 0
  {
    const auto & vpp_vectors = _fe_problem.getVectorPostprocessorVectors(_vpp_name);

    _stat_type_vector.clear();

    // Add the stat IDs into the first colum
    for (const auto & stat : _stats)
    {
      auto stat_id = stat.id();

      _stat_type_vector.push_back(stat_id);
    }

    // For each value vector compute the stats
    for (auto & the_pair : vpp_vectors)
    {
      const auto & name = the_pair.first;
      const auto & values = *the_pair.second.current;

      mooseAssert(_stat_vectors.count(name), "Error retrieving VPP vector");
      auto & stat_vector = *_stat_vectors.at(name);

      stat_vector.clear();

      for (const auto & stat : _stats)
      {
        auto stat_id = stat.id();

        stat_vector.push_back(computeStatValue(stat_id, values));
      }
    }
  }
}

void
StatisticsVectorPostprocessor::finalize()
{
}

Real
StatisticsVectorPostprocessor::computeStatValue(int stat_id, const std::vector<Real> & stat_vector)
{
  switch (stat_id)
  {
    case 0: // min
      return *std::min_element(stat_vector.begin(), stat_vector.end());
    case 1: // max
      return *std::max_element(stat_vector.begin(), stat_vector.end());
    case 2: // sum
      return std::accumulate(stat_vector.begin(), stat_vector.end(), 0.);
    case 3: // average
      return std::accumulate(stat_vector.begin(), stat_vector.end(), 0.) /
             static_cast<Real>(stat_vector.size());
    case 4: // stddev
    {
      auto mean = std::accumulate(stat_vector.begin(), stat_vector.end(), 0.) /
                  static_cast<Real>(stat_vector.size());

      auto the_sum = std::accumulate(stat_vector.begin(),
                                     stat_vector.end(),
                                     0.,
                                     [&mean](Real running_value, Real current_value) {
                                       return running_value + std::pow(current_value - mean, 2);
                                     });

      return std::sqrt(the_sum / (stat_vector.size() - 1.));
    }
    case 5: // norm2
      return std::sqrt(std::accumulate(
          stat_vector.begin(), stat_vector.end(), 0., [](Real running_value, Real current_value) {
            return running_value + std::pow(current_value, 2);

          }));
    default:
      mooseError("Unknown statistics type: ", stat_id);
  }
}
