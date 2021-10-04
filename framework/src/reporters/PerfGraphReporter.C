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
  : GeneralReporter(parameters),
    _nodes(declareValueByName<std::vector<NodeEntry>>("nodes", REPORTER_MODE_DISTRIBUTED))
{
}

void
PerfGraphReporter::execute()
{
  // PerfNodes do not have unique IDs; their ID is based on the section
  // in which they are generated. But... we want unique IDs. Hence this map
  std::size_t next_unique_id = 0;
  std::map<const PerfNode *, std::size_t> unique_ids;
  const auto unique_id = [&unique_ids, &next_unique_id](const PerfNode & node) {
    const auto find = unique_ids.find(&node);
    if (find != unique_ids.end())
      return find->second;
    unique_ids[&node] = next_unique_id;
    return next_unique_id++;
  };

  // Action that stores the nodes information info _nodes
  const auto add_entry = [this, &unique_id](const PerfGraph::PerfNodeInfo & info) {
    NodeEntry entry;
    entry.name = info.sectionInfo().name();
    entry.ints["id"] = unique_id(info.node());
    entry.ints["level"] = info.sectionInfo().level();
    entry.ints["num_calls"] = info.node().numCalls();
    entry.ints["memory"] = info.node().selfMemory();
    entry.time = info.node().selfTimeSec();

    if (info.parentNode())
      entry.ints["parent_id"] = unique_id(*info.parentNode());

    for (const auto & id_node_pair : info.node().children())
      entry.children_ids.push_back(unique_id(*id_node_pair.second));

    _nodes.push_back(entry);
  };

  // Fill _nodes for to_json
  _nodes.clear();
  _perf_graph.update();
  _perf_graph.treeRecurse(add_entry);
}

void
to_json(nlohmann::json & json, const PerfGraphReporter::NodeEntry & entry)
{
  json["name"] = entry.name;
  json["time"] = entry.time;
  for (const auto & name_val_pair : entry.ints)
    json[name_val_pair.first] = name_val_pair.second;
  if (entry.children_ids.size())
    json["children_ids"] = entry.children_ids;
}

void
dataStore(std::ostream & stream, PerfGraphReporter::NodeEntry & entry, void * context)
{
  dataStore(stream, entry.name, context);
  dataStore(stream, entry.ints, context);
  dataStore(stream, entry.time, context);
  dataStore(stream, entry.children_ids, context);
}
void
dataLoad(std::istream & stream, PerfGraphReporter::NodeEntry & entry, void * context)
{
  dataLoad(stream, entry.name, context);
  dataLoad(stream, entry.ints, context);
  dataLoad(stream, entry.time, context);
  dataLoad(stream, entry.children_ids, context);
}
