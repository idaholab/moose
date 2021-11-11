//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfNode.h"
#include "PerfGraph.h"

#include "DataIO.h"

std::chrono::steady_clock::duration
PerfNode::selfTime() const
{
  return _total_time - childrenTime();
}

std::chrono::steady_clock::duration
PerfNode::totalTime() const
{
  // Note that all of the children's time is already
  // accounted for in the total time
  return _total_time;
}

std::chrono::steady_clock::duration
PerfNode::childrenTime() const
{
  std::chrono::steady_clock::duration children_time(0);

  for (auto & child_it : _children)
    children_time += child_it.second->totalTime();

  return children_time;
}

long int
PerfNode::selfMemory() const
{
  return _total_memory - childrenMemory();
}

long int
PerfNode::childrenMemory() const
{
  long int children_memory = 0;

  for (auto & child_it : _children)
    children_memory += child_it.second->totalMemory();

  return children_memory;
}

void
dataStore(std::ostream & stream, const std::unique_ptr<PerfNode> & node, void *)
{
  // We store the name instead of the ID because the ID could change in recover
  std::string name = moose::internal::getPerfGraphRegistry().sectionInfo(node->id())._name;
  dataStore(stream, name, nullptr);

  dataStore(stream, node->_total_time, nullptr);
  dataStore(stream, node->_num_calls, nullptr);
  dataStore(stream, node->_total_memory, nullptr);

  // Recursively add all of the children
  std::size_t num_children = node->children().size();
  dataStore(stream, num_children, nullptr);
  for (auto & id_child_pair : node->_children)
  {
    const auto & child = id_child_pair.second;
    dataStore(stream, child, nullptr);
  }
}

void
dataLoad(std::istream & stream, const std::unique_ptr<PerfNode> & node, void * perf_graph)
{
  std::string name;
  // When we recursively add children, we grab the name before recursing into
  // dataLoad(), so only load the name if we're on the root
  if (node.get() == &static_cast<PerfGraph *>(perf_graph)->rootNode())
    dataLoad(stream, name, nullptr);

  std::chrono::steady_clock::duration total_time;
  dataLoad(stream, total_time, nullptr);
  node->_total_time += total_time;

  long unsigned int num_calls;
  dataLoad(stream, num_calls, nullptr);
  node->_num_calls += num_calls;

  long unsigned int total_memory;
  dataLoad(stream, total_memory, nullptr);
  node->_total_memory += total_memory;

  // Recursively add the children
  // If a matching child exists with the same name, the time/calls/memory will be appended
  // to said node. If a node does not exist with the same name, it will be created
  std::size_t num_children;
  dataLoad(stream, num_children, nullptr);
  std::size_t i = 0;
  while (i++ < num_children)
  {
    dataLoad(stream, name, nullptr);
    const auto id = moose::internal::getPerfGraphRegistry().sectionID(name);
    node->getChild(id); // creates the child if it does not exist

    const auto & child = node->_children[id];
    dataLoad(stream, child, perf_graph);
  }
}
