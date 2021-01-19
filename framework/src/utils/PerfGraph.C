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

// libMesh Includes
#include "libmesh/auto_ptr.h"

// System Includes
#include <chrono>

const std::string PerfGraph::ROOT_NAME = "Root";

PerfGraph::PerfGraph(const std::string & root_name)
  : _root_name(root_name), _current_position(0), _active(true)
{
  // Not done in the initialization list on purpose because this object needs to be complete first
  _root_node = libmesh_make_unique<PerfNode>(registerSection(ROOT_NAME, 0));

  // Set the initial time
  _root_node->setStartTime(std::chrono::steady_clock::now());

  // Add a call
  _root_node->incrementNumCalls();

  _stack[0] = _root_node.get();
}

unsigned int
PerfGraph::registerSection(const std::string & section_name, unsigned int level)
{
  auto it = _section_name_to_id.lower_bound(section_name);

  // Is it already registered?
  if (it != _section_name_to_id.end() && it->first == section_name)
    return it->second;

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

unsigned long int
PerfGraph::getNumCalls(const std::string & section_name)
{
  updateTiming();

  auto section_it = _section_time.find(section_name);

  if (section_it == _section_time.end())
  {
    // The section exists but has not ran yet, in which case we can return zero
    if (_section_name_to_id.count(section_name))
      return 0;

    mooseError(
        "Unknown section_name: ",
        section_name,
        " in PerfGraph::getNumCalls()\nIf you are attempting to retrieve the root use \"Root\".");
  }

  return section_it->second._num_calls;
}

Real
PerfGraph::getTime(const TimeType type, const std::string & section_name)
{
  updateTiming();

  auto section_it = _section_time.find(section_name);

  if (section_it == _section_time.end())
  {
    // The section exists but has not ran yet, in which case we can return zero
    if (_section_name_to_id.count(section_name))
      return 0;

    mooseError(
        "Unknown section_name: ",
        section_name,
        " in PerfGraph::getTime()\nIf you are attempting to retrieve the root use \"Root\".");
  }

  auto app_time = _section_time_ptrs[0]->_total;

  switch (type)
  {
    case SELF:
      return section_it->second._self;
    case CHILDREN:
      return section_it->second._children;
    case TOTAL:
      return section_it->second._total;
    case SELF_AVG:
      return section_it->second._self / static_cast<Real>(section_it->second._num_calls);
    case CHILDREN_AVG:
      return section_it->second._children / static_cast<Real>(section_it->second._num_calls);
    case TOTAL_AVG:
      return section_it->second._total / static_cast<Real>(section_it->second._num_calls);
    case SELF_PERCENT:
      return 100. * (section_it->second._self / app_time);
    case CHILDREN_PERCENT:
      return 100. * (section_it->second._children / app_time);
    case TOTAL_PERCENT:
      return 100. * (section_it->second._total / app_time);
    default:
      ::mooseError("Unknown TimeType");
  }
}

void
PerfGraph::push(const PerfID id)
{
  if (!_active)
    return;

  auto new_node = _stack[_current_position]->getChild(id);

  // Set the start time
  new_node->setStartTime(std::chrono::steady_clock::now());

  // Increment the number of calls
  new_node->incrementNumCalls();

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
PerfGraph::updateTiming()
{
  // First update all of the currently running nodes
  auto now = std::chrono::steady_clock::now();
  for (unsigned int i = 0; i <= _current_position; i++)
  {
    auto node = _stack[i];
    node->addTime(now);
    node->setStartTime(now);
  }

  // Zero out the entries
  for (auto & section_time_it : _section_time)
  {
    auto & section_time = section_time_it.second;

    section_time._num_calls = 0;
    section_time._self = 0.;
    section_time._children = 0.;
    section_time._total = 0.;
  }

  recursivelyFillTime(_root_node.get());

  // Update vector pointing to section times
  // Note: we are doing this _after_ recursively filling
  // because new entries may have been created
  _section_time_ptrs.resize(_id_to_section_name.size());

  for (auto & section_time_it : _section_time)
  {
    auto id = _section_name_to_id[section_time_it.first];

    _section_time_ptrs[id] = &section_time_it.second;
  }
}

void
PerfGraph::recursivelyFillTime(PerfNode * current_node)
{
  auto id = current_node->id();

  auto self = std::chrono::duration<double>(current_node->selfTime()).count();
  auto children = std::chrono::duration<double>(current_node->childrenTime()).count();
  auto total = std::chrono::duration<double>(current_node->totalTime()).count();
  auto num_calls = current_node->numCalls();

  // RHS insertion on purpose
  auto & section_time = _section_time[_id_to_section_name[id]];

  section_time._self += self;
  section_time._children += children;
  section_time._total += total;
  section_time._num_calls += num_calls;

  for (auto & child_it : current_node->children())
    recursivelyFillTime(child_it.second.get());
}

void
PerfGraph::recursivelyPrintGraph(PerfNode * current_node,
                                 FullTable & vtable,
                                 unsigned int level,
                                 unsigned int current_depth)
{
  mooseAssert(_id_to_section_name.find(current_node->id()) != _id_to_section_name.end(),
              "Unable to find section name!");

  auto & name = current_node->id() == 0 ? _root_name : _id_to_section_name[current_node->id()];

  mooseAssert(_id_to_level.find(current_node->id()) != _id_to_level.end(), "Unable to find level!");
  auto & node_level = _id_to_level[current_node->id()];

  if (node_level <= level)
  {
    mooseAssert(!_section_time_ptrs.empty(),
                "updateTiming() must be run before recursivelyPrintGraph!");

    auto section = std::string(current_depth * 2, ' ') + name;

    // The total time of the root node
    auto total_root_time = _section_time_ptrs[0]->_total;

    auto num_calls = current_node->numCalls();
    auto self = std::chrono::duration<double>(current_node->selfTime()).count();
    auto self_avg = self / static_cast<Real>(num_calls);
    auto self_percent = 100. * self / total_root_time;

    auto children = std::chrono::duration<double>(current_node->childrenTime()).count();
    auto children_avg = children / static_cast<Real>(num_calls);
    auto children_percent = 100. * children / total_root_time;

    auto total = std::chrono::duration<double>(current_node->totalTime()).count();
    auto total_avg = total / static_cast<Real>(num_calls);
    auto total_percent = 100. * total / total_root_time;

    vtable.addRow(section,
                  num_calls,
                  self,
                  self_avg,
                  self_percent,
                  children,
                  children_avg,
                  children_percent,
                  total,
                  total_avg,
                  total_percent);

    current_depth++;
  }

  for (auto & child_it : current_node->children())
    recursivelyPrintGraph(child_it.second.get(), vtable, level, current_depth);
}

void
PerfGraph::recursivelyPrintHeaviestGraph(PerfNode * current_node,
                                         FullTable & vtable,
                                         unsigned int current_depth)
{
  mooseAssert(!_section_time_ptrs.empty(),
              "updateTiming() must be run before recursivelyPrintGraph!");

  auto & name = current_node->id() == 0 ? _root_name : _id_to_section_name[current_node->id()];

  auto section = std::string(current_depth * 2, ' ') + name;

  // The total time of the root node
  auto total_root_time = _section_time_ptrs[0]->_total;

  auto num_calls = current_node->numCalls();
  auto self = std::chrono::duration<double>(current_node->selfTime()).count();
  auto self_avg = self / static_cast<Real>(num_calls);
  auto self_percent = 100. * self / total_root_time;

  auto children = std::chrono::duration<double>(current_node->childrenTime()).count();
  auto children_avg = children / static_cast<Real>(num_calls);
  auto children_percent = 100. * children / total_root_time;

  auto total = std::chrono::duration<double>(current_node->totalTime()).count();
  auto total_avg = total / static_cast<Real>(num_calls);
  auto total_percent = 100. * total / total_root_time;

  vtable.addRow(section,
                num_calls,
                self,
                self_avg,
                self_percent,
                children,
                children_avg,
                children_percent,
                total,
                total_avg,
                total_percent);

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
  updateTiming();

  console << "\nPerformance Graph:\n";
  FullTable vtable({"Section",
                    "Calls",
                    "Self(s)",
                    "Avg(s)",
                    "%",
                    "Children(s)",
                    "Avg(s)",
                    "%",
                    "Total(s)",
                    "Avg(s)",
                    "%"},
                   10);

  vtable.setColumnFormat({VariadicTableColumnFormat::AUTO,      // Section Name
                          VariadicTableColumnFormat::AUTO,      // Calls
                          VariadicTableColumnFormat::FIXED,     // Self
                          VariadicTableColumnFormat::FIXED,     // Avg.
                          VariadicTableColumnFormat::PERCENT,   // %
                          VariadicTableColumnFormat::FIXED,     // Children
                          VariadicTableColumnFormat::FIXED,     // Avg.
                          VariadicTableColumnFormat::PERCENT,   // %
                          VariadicTableColumnFormat::FIXED,     // Total
                          VariadicTableColumnFormat::FIXED,     // Avg.
                          VariadicTableColumnFormat::PERCENT}); // %

  vtable.setColumnPrecision({1, 0, 3, 3, 2, 3, 3, 2, 3, 3, 2});

  recursivelyPrintGraph(_root_node.get(), vtable, level);
  vtable.print(console);
}

void
PerfGraph::printHeaviestBranch(const ConsoleStream & console)
{
  updateTiming();

  console << "\nHeaviest Branch:\n";
  FullTable vtable({"Section",
                    "Calls",
                    "Self(s)",
                    "Avg(s)",
                    "%",
                    "Children(s)",
                    "Avg(s)",
                    "%",
                    "Total(s)",
                    "Avg(s)",
                    "%"},
                   10);

  vtable.setColumnFormat({VariadicTableColumnFormat::AUTO,      // Section Name
                          VariadicTableColumnFormat::AUTO,      // Calls
                          VariadicTableColumnFormat::FIXED,     // Self
                          VariadicTableColumnFormat::FIXED,     // Avg.
                          VariadicTableColumnFormat::PERCENT,   // %
                          VariadicTableColumnFormat::FIXED,     // Children
                          VariadicTableColumnFormat::FIXED,     // Avg.
                          VariadicTableColumnFormat::PERCENT,   // %
                          VariadicTableColumnFormat::FIXED,     // Total
                          VariadicTableColumnFormat::FIXED,     // Avg.
                          VariadicTableColumnFormat::PERCENT}); // %

  vtable.setColumnPrecision({1, 0, 3, 3, 2, 3, 3, 2, 3, 3, 2});

  recursivelyPrintHeaviestGraph(_root_node.get(), vtable);
  vtable.print(console);
}

void
PerfGraph::printHeaviestSections(const ConsoleStream & console, const unsigned int num_sections)
{
  updateTiming();

  console << "\nHeaviest Sections:\n";

  // Indirect Sort The Self Time
  std::vector<size_t> sorted;
  Moose::indirectSort(_section_time_ptrs.begin(),
                      _section_time_ptrs.end(),
                      sorted,
                      [](SectionTime * lhs, SectionTime * rhs) {
                        if (lhs && rhs)
                          return lhs->_self > rhs->_self;

                        // If the LHS exists - it's definitely bigger than a non-existant RHS
                        if (lhs)
                          return true;

                        // Both don't exist - so it doesn't matter how we sort them
                        return false;
                      });

  HeaviestTable vtable({"Section", "Calls", "Self(s)", "Avg.", "%"}, 10);

  vtable.setColumnFormat({VariadicTableColumnFormat::AUTO, // Doesn't matter
                          VariadicTableColumnFormat::AUTO,
                          VariadicTableColumnFormat::FIXED,
                          VariadicTableColumnFormat::FIXED,
                          VariadicTableColumnFormat::PERCENT});

  vtable.setColumnPrecision({1, 1, 3, 3, 2});

  mooseAssert(!_section_time_ptrs.empty(),
              "updateTiming() must be run before printHeaviestSections()!");

  // The total time of the root node
  auto total_root_time = _section_time_ptrs[0]->_total;

  // Now print out the largest ones
  for (unsigned int i = 0; i < num_sections; i++)
  {
    auto id = sorted[i];

    vtable.addRow(id == 0 ? _root_name : _id_to_section_name[id],
                  _section_time_ptrs[id]->_num_calls,
                  _section_time_ptrs[id]->_self,
                  _section_time_ptrs[id]->_self /
                      static_cast<Real>(_section_time_ptrs[id]->_num_calls),
                  100 * _section_time_ptrs[id]->_self / total_root_time);
  }

  vtable.print(console);
}
