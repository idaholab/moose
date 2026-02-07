//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "AnyPointer.h"
#include "nlohmann/json.h"

#include <cstddef>
#include <vector>

class PerfNode;
class PerfGraph;

/**
 * Reports the full graph from the PerfGraph
 */
class PerfGraphReporter : public GeneralReporter
{
public:
  static InputParameters validParams();

  PerfGraphReporter(const InputParameters & parameters);

  void initialize() override {}
  void finalize() override;
  void execute() override {}

  /// The reporter values that this object declares; used within the
  /// CommonOutputAction to single out values from this reporter
  static const std::vector<ReporterValueName> value_names;

  /**
   * Structure that holds the PerfGraph in JSON.
   *
   * We build the JSON itself during execute() as it is
   * a complex tree and it's easier to not have an intermediate
   * data type.
   */
  struct PerfGraphJSON
  {
    nlohmann::json value;
  };

private:
  /// "graph" entry in the reporter; the actual graph
  PerfGraphJSON & _graph;
  /// "_max_memory_this_proc" entry in the reporter; max memory for this rank
  std::size_t & _max_memory_this_rank;
  /// "max_memory_per_proc" entry in the reporter; max memory across all ranks
  std::vector<std::size_t> & _max_memory_per_rank;
};

void to_json(nlohmann::json & json, const PerfGraphReporter::PerfGraphJSON & perf_graph_json);

/**
 * Store and load methods for const PerfGraph *, used in the PerfGraphReporter,
 * which do nothing.
 *
 * They do nothing because the recover capability is retained in the
 * PerfGraph itself, which will recover and append a previously ran
 * graph to the current PerfGraph.
 */
///@{
template <typename Context>
void dataStore(std::ostream &, PerfGraphReporter::PerfGraphJSON &, Context);
template <typename Context>
void dataLoad(std::istream &, PerfGraphReporter::PerfGraphJSON &, Context);
///@}
