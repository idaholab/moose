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

// Note: do everything we can to make sure this only gets #included
// in the .C file... this is a heavily templated header that we
// don't want to expose to EVERY file in MOOSE...
#include "VariadicTable.h"

PerfGraph::PerfGraph() : _current_position(0), _active(true)
{
  // Not done in the initialization list on purpose because this object needs to be complete first
  _root_node = libmesh_make_unique<PerfNode>(registerSection("App", 0));

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

void
PerfGraph::recursivelyPrintGraph(PerfNode * current_node,
                                 VariadicTable<std::string, Real, Real, Real> & vtable,
                                 unsigned int level,
                                 unsigned int current_depth)
{
  auto & name = _id_to_section_name[current_node->id()];
  auto & node_level = _id_to_level[current_node->id()];

  if (node_level <= level)
  {
    auto section = std::string(current_depth * 2, ' ') + name;
    vtable.addRow({section,
                   std::chrono::duration<double>(current_node->selfTime()).count(),
                   std::chrono::duration<double>(current_node->childrenTime()).count(),
                   std::chrono::duration<double>(current_node->totalTime()).count()});

    current_depth++;
  }

  for (auto & child_it : current_node->children())
    recursivelyPrintGraph(child_it.second.get(), vtable, level, current_depth);
}

void
PerfGraph::recursivelyPrintHeaviestGraph(PerfNode * current_node,
                                         VariadicTable<std::string, Real, Real, Real> & vtable,
                                         unsigned int current_depth)
{
  auto & name = _id_to_section_name[current_node->id()];

  auto section = std::string(current_depth * 2, ' ') + name;

  vtable.addRow({section,
                 std::chrono::duration<double>(current_node->selfTime()).count(),
                 std::chrono::duration<double>(current_node->childrenTime()).count(),
                 std::chrono::duration<double>(current_node->totalTime()).count()});

  current_depth++;

  if (!current_node->children().empty())
  {
    PerfNode * heaviest_child = nullptr;

    for (auto & child_it : current_node->children())
    {
      auto current_child = child_it.second.get();

      if (!heaviest_child || (current_child->totalTime() > heaviest_child->totalTime()))
        heaviest_child = current_child;
    }

    recursivelyPrintHeaviestGraph(heaviest_child, vtable, current_depth);
  }
}

void
PerfGraph::print(const ConsoleStream & console, unsigned int level)
{
  updateCurrentlyRunning();

  console << "\nPerformance Graph:\n";
  VariadicTable<std::string, Real, Real, Real> vtable(
      {"Section", "Self(s)", "Children(s)", "Total(s)"}, 13);
  recursivelyPrintGraph(_root_node.get(), vtable, level);
  vtable.print(console);
}

void
PerfGraph::printHeaviestBranch(const ConsoleStream & console)
{
  updateCurrentlyRunning();

  console << "\nHeaviest Branch:\n";
  VariadicTable<std::string, Real, Real, Real> vtable(
      {"Section", "Self(s)", "Children(s)", "Total(s)"}, 13);
  recursivelyPrintHeaviestGraph(_root_node.get(), vtable);
  vtable.print(console);
}

void
PerfGraph::printHeaviestSections(const ConsoleStream & console, const unsigned int num_sections)
{
  updateCurrentlyRunning();

  console << "\nHeaviest Sections:\n";

  // First - accumulate the time for each section
  std::vector<Real> section_self_time(_section_name_to_id.size(), 0.);
  recursivelyFillTotalSelfTime(_root_node.get(), section_self_time);

  // Now indirect sort them
  std::vector<size_t> sorted;
  Moose::indirectSort(section_self_time.begin(),
                      section_self_time.end(),
                      sorted,
                      [](double lhs, double rhs) { return lhs > rhs; });

  VariadicTable<std::string, Real> vtable({"Section", "Total Time(s)"}, 13);

  // Now print out the largest ones
  for (unsigned int i = 0; i < num_sections; i++)
  {
    auto id = sorted[i];

    vtable.addRow({_id_to_section_name[id], section_self_time[id]});
  }

  vtable.print(console);
}
