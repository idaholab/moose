//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGraphReporter.h"

#include "PerfGraph.h"

registerMooseObject("MooseApp", PerfGraphReporter);

const std::vector<ReporterValueName> PerfGraphReporter::value_names{
    "graph", "max_memory_this_rank", "max_memory_per_rank", "version"};

InputParameters
PerfGraphReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reports the full performance graph from the PerfGraph.");
  return params;
}

PerfGraphReporter::PerfGraphReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _graph(declareValueByName<PerfGraphJSON>("graph", REPORTER_MODE_DISTRIBUTED)),
    _max_memory_this_rank(
        declareValueByName<std::size_t>("max_memory_this_rank", REPORTER_MODE_DISTRIBUTED, 0)),
    _max_memory_per_rank(declareValueByName<std::vector<std::size_t>>(
        "max_memory_per_rank", REPORTER_MODE_ROOT, std::vector<std::size_t>()))
{
  // Store the version schema so at a later date we can use this version
  // to know what data we should expect. This value is never changed during
  // execute() as it is static and is saved as distributed so that we can
  // store off each processor's data individually and know of the version
  // without needing the context of the root process output.
  //
  // Version 0:
  // - Initial version when version wasn't set
  // Version 1:
  // - Store children in separate "children" key per node
  // - Removed "memory" key for each node
  // - Add "max_memory_this_rank" top-level key
  // - Add "max_memory_per_rank" top-level key
  declareValueByName<unsigned int>("version", REPORTER_MODE_DISTRIBUTED, 1);
}

void
PerfGraphReporter::finalize()
{
  auto & perf_graph = perfGraph();

  // Build the graph
  _graph.value.clear();
  // Function for recursing through nodes
  std::function<void(nlohmann::json & json, const PerfNode &)> recurse_node;
  recurse_node = [&recurse_node](nlohmann::json & json, const PerfNode & node)
  {
    const auto & info = moose::internal::getPerfGraphRegistry().sectionInfo(node.id());

    auto & node_json = json[info._name];
    node_json["level"] = info._level;
    node_json["num_calls"] = node.numCalls();
    node_json["time"] = node.selfTimeSec();

    // Recursively add the children
    if (node.children().size())
    {
      auto & children = node_json["children"];
      for (const auto & id_child_pair : node.children())
        recurse_node(children, *id_child_pair.second);
    }
  };
  // Update the timings in each node before we output them
  perf_graph.update();
  // Recurse through the nodes to add
  recurse_node(_graph.value, perf_graph.rootNode());

  // Memory on this rank
  _max_memory_this_rank = perf_graph.getMaxMemory();
  // Memory across all ranks on rank 0; we keep this separate so
  // that we can store data for rank 0 in a database but still
  // have a general idea of memory usage across all ranks
  comm().gather(0, _max_memory_this_rank, _max_memory_per_rank);
}

void
to_json(nlohmann::json & json, const PerfGraphReporter::PerfGraphJSON & perf_graph_json)
{
  json = perf_graph_json.value;
}

template <typename Context>
void
dataStore(std::ostream &, PerfGraphReporter::PerfGraphJSON &, Context)
{
}

template void dataStore(std::ostream &, PerfGraphReporter::PerfGraphJSON &, void *);
template void dataStore(std::ostream &, PerfGraphReporter::PerfGraphJSON &, std::nullptr_t);

template <typename Context>
void
dataLoad(std::istream &, PerfGraphReporter::PerfGraphJSON &, Context)
{
}

template void dataLoad(std::istream &, PerfGraphReporter::PerfGraphJSON &, void *);
template void dataLoad(std::istream &, PerfGraphReporter::PerfGraphJSON &, std::nullptr_t);
