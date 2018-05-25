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
PerfGraph::registerSection(const std::string & section_name)
{
  auto it = _section_name_to_id.lower_bound(section_name);

  // Is it already registered?
  if (it != _section_name_to_id.end() && it->first == section_name)
    mooseError("PerfGraph already holds a section named: ", section_name);

  // It's not...
  auto id = _section_name_to_id.size();
  _section_name_to_id.emplace_hint(it, section_name, id);
  _id_to_section_name[id] = section_name;

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
  auto new_node = _stack[_current_position]->getChild(id);

  _current_position++;

  if (_current_position >= MAX_STACK_SIZE)
    mooseError("PerfGraph is out of stack space!");

  _stack[_current_position] = new_node;
}

void
PerfGraph::pop(const std::chrono::steady_clock::duration duration)
{
  _stack[_current_position]->addTime(duration);

  _current_position--;
}

void
PerfGraph::updateRoot()
{
  auto now = std::chrono::steady_clock::now();

  _root_node->addTime(now - _root_start);

  _root_start = now;
}
