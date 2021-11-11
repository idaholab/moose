//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGraphReporter.h"

#include "PerfGraph.h"

registerMooseObject("MooseApp", PerfGraphReporter);

InputParameters
PerfGraphReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reports the full performance graph from the PerfGraph.");

  // Because the PerfGraphReporter does all of its execution within the
  // to_json specialization (does not fill the structure within execute()),
  // this allows us to obey the user's request for execute_on
  params.set<bool>("_always_store") = false;

  return params;
}

PerfGraphReporter::PerfGraphReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _graph(declareValueByName<const PerfGraph *>("graph", REPORTER_MODE_DISTRIBUTED, &perfGraph()))
{
}

void
to_json(nlohmann::json & json, const PerfGraph * const & perf_graph)
{
  mooseAssert(perf_graph, "perf_graph is not set");

  // Update the timings in each node before we output them
  const_cast<PerfGraph *>(perf_graph)->update();

  // Recurse through the nodes starting with the root to fill the graph
  to_json(json, perf_graph->rootNode());
}

void
to_json(nlohmann::json & json, const PerfNode & node)
{
  const auto & info = moose::internal::getPerfGraphRegistry().sectionInfo(node.id());

  auto & node_json = json[info._name];
  node_json["level"] = info._level;
  node_json["num_calls"] = node.numCalls();
  node_json["memory"] = node.selfMemory();
  node_json["time"] = node.selfTimeSec();

  // Recursively add the children
  for (const auto & id_child_pair : node.children())
    to_json(node_json, *id_child_pair.second);
}
