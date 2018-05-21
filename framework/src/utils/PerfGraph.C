//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGraph.h"

// MOOSE Includes
#include "PerfGuard.h"
#include "MooseError.h"

PerfGraph::PerfGraph() : _current_position(0), _active(true)
{
  // Not done in the initialization list on purpose because this object needs to be complete first
  _root_node = libmesh_make_unique<PerfNode>(registerSection("root"));

  _stack[0] = _root_node.get();

  _root_start = std::chrono::steady_clock::now();
}

PerfGraph::~PerfGraph() {}

unsigned int
PerfGraph::registerSection(std::string section_name)
{
  auto it = _section_name_to_id.lower_bound(section_name);

  // Is it already registered?
  if (it != _section_name_to_id.lower_bound(section_name) && it->first == section_name)
    return it->second;

  // It's not...
  auto id = _section_name_to_id.size();
  _section_name_to_id.emplace_hint(it, section_name, id);
  _id_to_section_name[id] = section_name;

  return id;
}

void
PerfGraph::push(PerfID id)
{
  auto new_node = _stack[_current_position]->getChild(id);

  _current_position++;

  if (_current_position >= MAX_STACK_SIZE)
    mooseError("PerfGraph is out of stack space!");

  _stack[_current_position] = new_node;
}

void
PerfGraph::pop(std::chrono::steady_clock::duration duration)
{
  _stack[_current_position]->addTime(duration);

  _current_position--;
}

/**
 * Helper function to recursively print out the graph
 */
void
recursivelyPrintGraph(PerfGraph & graph, PerfNode * current_node, unsigned int current_depth = 0)
{
  auto & name = graph.sectionName(current_node->id());

  std::cout << std::string(current_depth, ' ') << name
            << " self: " << std::chrono::duration<double>(current_node->selfTime()).count()
            << " children: " << std::chrono::duration<double>(current_node->childrenTime()).count()
            << " total: " << std::chrono::duration<double>(current_node->totalTime()).count()
            << "\n";

  for (auto & child_it : current_node->children())
    recursivelyPrintGraph(graph, child_it.second.get(), current_depth + 1);
}

void
PerfGraph::print()
{
  updateRoot();
  recursivelyPrintGraph(*this, _root_node.get());
}

std::string &
PerfGraph::sectionName(PerfID id)
{
  auto & name = _id_to_section_name.at(id);

  return name;
}

void
PerfGraph::updateRoot()
{
  auto now = std::chrono::steady_clock::now();

  _root_node->addTime(now - _root_start);

  _root_start = now;
}
