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
  return params;
}

PerfGraphReporter::PerfGraphReporter(const InputParameters & parameters)
  : GeneralReporter(parameters)
{
  declareValueByName<const PerfGraph *>("perf_graph", REPORTER_MODE_DISTRIBUTED) = &_perf_graph;
}

void
to_json(nlohmann::json & json, const PerfGraph * const & perf_graph)
{
  const_cast<PerfGraph *>(perf_graph)->updateTiming();
  auto act = [&json](const PerfGraph::PerfNodeInfo & info) {
    nlohmann::json entry;
    entry["id"] = info.node().id();
    entry["name"] = info.sectionInfo().name();
    entry["level"] = info.sectionInfo().level();
    entry["num_calls"] = info.node().numCalls();

    entry["time"] = info.node().selfTimeSec();
    entry["memory"] = info.node().selfMemory();

    if (info.node().children().size())
    {
      entry["children_ids"] = std::vector<PerfID>();
      for (const auto & id_node_pair : info.node().children())
        entry["children_ids"].push_back(id_node_pair.first);
    }

    if (info.parentNode())
      entry["parent_id"] = info.parentNode()->id();

    json.push_back(entry);
  };
  perf_graph->treeRecurse(act);
}
