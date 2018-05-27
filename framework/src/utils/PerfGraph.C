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
  _root_node = libmesh_make_unique<PerfNode>(registerSection("Root", 0));

  // Set the initial time
  _root_node->setStartTime(std::chrono::steady_clock::now());

  _stack[0] = _root_node.get();
}

PerfGraph::~PerfGraph() {}

unsigned int
PerfGraph::registerSection(const std::string & section_name, unsigned int level)
{
  auto it = _section_name_to_id.lower_bound(section_name);

  // Is it already registered?
  if (it != _section_name_to_id.end() && it->first == section_name)
    mooseError("PerfGraph already holds a section named: ", section_name);

  // It's not...
  auto id = _section_name_to_id.size();
  _section_name_to_id.emplace_hint(it, section_name, id);
  _id_to_section_name[id] = section_name;
  _id_to_level[id] = level;

  return id;
}

const std::string &
PerfGraph::sectionName(const PerfID id) const
{
  auto find_it = _id_to_section_name.find(id);

  if (find_it == _id_to_section_name.end())
    mooseError("PerfGraph cannot find a section name associated with id: ", id);

  return find_it->second;
}

void
PerfGraph::push(const PerfID id)
{
  if (!_active)
    return;

  auto new_node = _stack[_current_position]->getChild(id);

  // Set the start time
  new_node->setStartTime(std::chrono::steady_clock::now());

  _current_position++;

  if (_current_position >= MAX_STACK_SIZE)
    mooseError("PerfGraph is out of stack space!");

  _stack[_current_position] = new_node;
}

void
PerfGraph::pop()
{
  if (!_active)
    return;

  _stack[_current_position]->addTime(std::chrono::steady_clock::now());

  _current_position--;
}

void
PerfGraph::updateCurrentlyRunning()
{
  auto now = std::chrono::steady_clock::now();

  for (unsigned int i = 0; i <= _current_position; i++)
  {
    auto node = _stack[i];
    node->addTime(now);
    node->setStartTime(now);
  }
}

void
PerfGraph::recursivelyFillTotalSelfTime(PerfNode * current_node,
                                        std::vector<Real> & section_self_time)
{
  auto id = current_node->id();

  auto time = std::chrono::duration<double>(current_node->selfTime()).count();

  section_self_time[id] += time;

  for (auto & child_it : current_node->children())
    recursivelyFillTotalSelfTime(child_it.second.get(), section_self_time);
}
