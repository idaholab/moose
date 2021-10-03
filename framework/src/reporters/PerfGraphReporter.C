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
  // We store the value as a PerfGraph * here because we can't use an actual PerfGraph - the
  // reporter system will try to default construct a PerfGraph. Upon restart/recover, this
  // the type const PerfGraph * will be loaded as nullptr because it doesn't really make
  // sense to keep the PerfGraph state (as of yet...). We don't store the reporter value
  // here because we don't actually need it - the filling action is performed all in the
  // to_json method for const PerfGraph *
  declareValueByName<const PerfGraph *>("perf_graph", REPORTER_MODE_DISTRIBUTED) = &_perf_graph;
}

void
to_json(nlohmann::json & json, const PerfGraph * const & perf_graph)
{
  // Must update the timing before filling so that the data in each
  // PerfNode is up to date
  const_cast<PerfGraph *>(perf_graph)->update();

  // PerfNodes do not have unique IDs; their ID is based on the section
  // in which they are generated. But... we want unique IDs. Hence this map
  std::size_t next_unique_id = 0;
  std::map<const PerfNode *, std::size_t> unique_ids;
  auto unique_id = [&unique_ids, &next_unique_id](const PerfNode & node) {
    const auto find = unique_ids.find(&node);
    if (find != unique_ids.end())
      return find->second;
    unique_ids[&node] = next_unique_id;
    return next_unique_id++;
  };

  // Leverage the treeRecurse() method in PerfGraph, which will recurse
  // through the graph and act on each entry. In this case, for each entry
  // in the graph we add a single entry along with the parent/children
  // info so that the graph can be rebuilt in postprocessing
  auto & graph = json["graph"];
  auto act = [&graph, &unique_id](const PerfGraph::PerfNodeInfo & info) {
    nlohmann::json entry;
    entry["id"] = unique_id(info.node());
    entry["name"] = info.sectionInfo().name();
    entry["level"] = info.sectionInfo().level();
    entry["num_calls"] = info.node().numCalls();

    entry["time"] = info.node().selfTimeSec();
    entry["memory"] = info.node().selfMemory();

    if (info.node().children().size())
    {
      entry["children_ids"] = std::vector<PerfID>();
      for (const auto & id_node_pair : info.node().children())
        entry["children_ids"].push_back(unique_id(*id_node_pair.second));
    }

    if (info.parentNode())
      entry["parent_id"] = unique_id(*info.parentNode());

    graph.push_back(entry);
  };
  perf_graph->treeRecurse(act);
}

// We don't currently support loading PerfGraph state in
// restart/recover, so make that abundantly clear
void
dataStore(std::ostream &, const PerfGraph *&, void *)
{
}
void
dataLoad(std::istream &, const PerfGraph *& perf_graph, void *)
{
  perf_graph = nullptr;
}
