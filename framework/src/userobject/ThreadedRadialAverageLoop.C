//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThreadedRadialAverageLoop.h"
#include "Function.h"

ThreadedRadialAverageLoop::ThreadedRadialAverageLoop(RadialAverage & green) : _radavg(green) {}

// Splitting Constructor
ThreadedRadialAverageLoop::ThreadedRadialAverageLoop(const ThreadedRadialAverageLoop & x,
                                                     Threads::split /*split*/)
  : _radavg(x._radavg)
{
}

void
ThreadedRadialAverageLoop::operator()(const QPDataRange & qpdata_range)
{
  // fetch data from parent
  const auto radius = _radavg._radius;
  const auto & qp_data = _radavg._qp_data;
  const auto & kd_tree = _radavg._kd_tree;
  const auto & weights_type = _radavg._weights_type;

  // tree search data structures
  std::vector<std::pair<std::size_t, Real>> ret_matches;
  nanoflann::SearchParams search_params;

  // result map entry
  const auto end_it = _radavg._average.end();
  auto it = end_it;

  // iterate over qp range
  for (auto && local_qp : qpdata_range)
  {
    // Look up result map iterator only if we enter a new element. this saves a bunch
    // of map lookups because same element entries are consecutive in the qp_data vector.
    if (it == end_it || it->first != local_qp._elem_id)
      it = _radavg._average.find(local_qp._elem_id);

    // initialize result entry
    mooseAssert(it != end_it, "Current element id not found in result set.");
    auto & sum = it->second[local_qp._qp];
    sum = 0.0;

    ret_matches.clear();
    std::size_t n_result =
        kd_tree->radiusSearch(&(local_qp._q_point(0)), radius * radius, ret_matches, search_params);
    Real total_vol = 0.0;
    Real weight = 1.0;
    for (std::size_t j = 0; j < n_result; ++j)
    {
      const auto & other_qp = qp_data[ret_matches[j].first];
      switch (weights_type)
      {
        case RadialAverage::WeightsType::CONSTANT:
          break;

        case RadialAverage::WeightsType::LINEAR:
          weight = radius - std::sqrt(ret_matches[j].second);
          break;

        case RadialAverage::WeightsType::COSINE:
          weight = std::cos(std::sqrt(ret_matches[j].second) / radius * libMesh::pi) + 1.0;
          break;
      }

      sum += other_qp._value * other_qp._volume * weight;
      total_vol += other_qp._volume * weight;
    }
    sum /= total_vol;
  }
}
