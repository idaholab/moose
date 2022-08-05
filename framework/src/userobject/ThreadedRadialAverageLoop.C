/**********************************************************************/
/*                     DO NOT MODIFY THIS HEADER                      */
/* MAGPIE - Mesoscale Atomistic Glue Program for Integrated Execution */
/*                                                                    */
/*            Copyright 2017 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

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
  const auto r_cut = _radavg._r_cut;
  const auto & qp_data = _radavg._qp_data;
  const auto & kd_tree = _radavg._kd_tree;

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
        kd_tree->radiusSearch(&(local_qp._q_point(0)), r_cut, ret_matches, search_params);
    Real total_vol = 0.0;
    for (std::size_t j = 0; j < n_result; ++j)
    {
      const auto & other_qp = qp_data[ret_matches[j].first];
      sum += other_qp._value * other_qp._volume;
      total_vol += other_qp._volume;
    }
    sum /= total_vol;
  }
}
