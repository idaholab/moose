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
#include "PerfGraphLivePrint.h"
#include "MooseApp.h"

// Note: do everything we can to make sure this only gets #included
// in the .C file... this is a heavily templated header that we
// don't want to expose to EVERY file in MOOSE...
#include "VariadicTable.h"

// libMesh Includes
#include "libmesh/auto_ptr.h"

// System Includes
#include <chrono>

PerfGraph::PerfGraph(const std::string & root_name,
                     MooseApp & app,
                     const bool live_all,
                     const bool perf_graph_live)
  : ConsoleStreamInterface(app),
    _live_print_all(live_all),
    _disable_live_print(!perf_graph_live),
    _perf_graph_registry(moose::internal::getPerfGraphRegistry()),
    _pid(app.processor_id()),
    _current_position(-1),
    _stack(),
    _execution_list_begin(0),
    _execution_list_end(0),
    _section_name_to_id(_perf_graph_registry._section_name_to_id),
    _id_to_section_info(_perf_graph_registry._id_to_section_info),
    _active(true),
    _live_print_active(true),
    _destructing(false),
    _live_print(std::make_unique<PerfGraphLivePrint>(*this, app)),
    _live_print_time_limit(1.0),
    _live_print_mem_limit(100)
{
  if (_pid == 0 && !_disable_live_print)
  {
    // Start the printing thread
    _print_thread = std::thread([this] { this->_live_print->start(); });
  }

  _root_name = root_name;
  _root_node_id = _perf_graph_registry.registerSection(root_name, 0);

  push(_root_node_id);
}

PerfGraph::~PerfGraph() { disableLivePrint(); }

const std::string &
PerfGraph::sectionName(const PerfID id) const
{
  auto find_it = _id_to_section_info.find(id);

  if (find_it == _id_to_section_info.end())
    mooseError("PerfGraph cannot find a section name associated with id: ", id);

  return find_it->second._name;
}

void
PerfGraph::disableLivePrint()
{
  if (_pid == 0 && !_disable_live_print)
  {
    _done.set_value(true);

    _destructing = true;

    _finished_section.notify_one();

    _print_thread.join();

    _disable_live_print = true;
  }
}

unsigned long int
PerfGraph::getNumCalls(const std::string & section_name)
{
  updateTiming();

  std::string real_section_name = section_name;

  if (section_name == "Root")
    real_section_name = _root_name;

  auto section_it = _section_time.find(real_section_name);

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

  std::string real_section_name = section_name;

  if (section_name == "Root")
    real_section_name = _root_name;

  auto section_it = _section_time.find(real_section_name);

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

  auto app_time = _section_time_ptrs[_root_node_id]->_total;

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
    case SELF_MEMORY:
      return section_it->second._self_memory;
    case CHILDREN_MEMORY:
      return section_it->second._children_memory;
    case TOTAL_MEMORY:
      return section_it->second._total_memory;
    default:
      ::mooseError("Unknown TimeType");
  }
}

void
PerfGraph::addToExecutionList(const PerfID id,
                              const IncrementState state,
                              const std::chrono::time_point<std::chrono::steady_clock> time,
                              const long int memory)
{
  auto & section_increment = _execution_list[_execution_list_end];

  section_increment._id = id;
  section_increment._state = state;
  section_increment._time = time;
  section_increment._memory = memory;

  // This will synchronize the above memory changes with the
  // atomic_thread_fence in the printing thread
  std::atomic_thread_fence(std::memory_order_release);

  // All of the above memory operations will be seen by the
  // printing thread before the printing thread sees this new value
  //
  // Note fetch_add is a _post_-increment operation.  The +1 here is to add to the previous value
  // so we can get the next value.
  auto next_execution_list_end = _execution_list_end.fetch_add(1, std::memory_order_relaxed) + 1;

  // Are we at the end of our circular buffer?
  if (next_execution_list_end >= MAX_EXECUTION_LIST_SIZE)
    _execution_list_end.store(0, std::memory_order_relaxed);
}

void
PerfGraph::push(const PerfID id)
{
  if (!_active && !_live_print_active)
    return;

  PerfNode * new_node = nullptr;

  if (_current_position == -1) // Must be the root node - need to makew it
  {
    _root_node = libmesh_make_unique<PerfNode>(id);
    new_node = _root_node.get();
  }
  else
    new_node = _stack[_current_position]->getChild(id);

  MemoryUtils::Stats stats;
  MemoryUtils::getMemoryStats(stats);
  auto start_memory =
      MemoryUtils::convertBytes(stats._physical_memory, MemoryUtils::MemUnits::Megabytes);

  // Set the start time
  auto current_time = std::chrono::steady_clock::now();

  new_node->setStartTimeAndMemory(current_time, start_memory);

  // Increment the number of calls
  new_node->incrementNumCalls();

  _current_position++;

  if (_current_position >= MAX_STACK_SIZE)
    mooseError("PerfGraph is out of stack space!");

  _stack[_current_position] = new_node;

  // Add this to the execution list
  if ((_live_print_active || _live_print_all) && (_pid == 0 && !_disable_live_print) &&
      (!_id_to_section_info[id]._live_message.empty() || _live_print_all))
  {
    addToExecutionList(id, IncrementState::started, current_time, start_memory);
  }
}

void
PerfGraph::pop()
{
  if (!_active && !_live_print_active)
    return;

  MemoryUtils::Stats stats;
  MemoryUtils::getMemoryStats(stats);
  auto current_memory =
      MemoryUtils::convertBytes(stats._physical_memory, MemoryUtils::MemUnits::Megabytes);

  auto current_time = std::chrono::steady_clock::now();

  auto & current_node = _stack[_current_position];

  current_node->addTimeAndMemory(current_time, current_memory);

  _current_position--;

  // Add this to the exection list
  if ((_live_print_active || _live_print_all) && (_pid == 0 && !_disable_live_print) &&
      (!_id_to_section_info[current_node->id()]._live_message.empty() || _live_print_all))
  {
    addToExecutionList(current_node->id(), IncrementState::finished, current_time, current_memory);

    // Tell the printing thread that a section has finished
    //
    // Note: no mutex is needed here because we're using an atomic
    // in the predicate of the condition_variable in the thread
    // This is technically correct - but there is a chance of missing a signal
    // For us - that chance is low and doesn't matter (the timeout will just be hit
    // instead). So - I would rather not have an extra lock here in the main thread.
    _finished_section.notify_one();
  }
}

void
PerfGraph::updateTiming()
{
  // First update all of the currently running nodes
  auto now = std::chrono::steady_clock::now();

  MemoryUtils::Stats stats;
  MemoryUtils::getMemoryStats(stats);
  auto now_memory =
      MemoryUtils::convertBytes(stats._physical_memory, MemoryUtils::MemUnits::Megabytes);

  for (int i = 0; i <= _current_position; i++)
  {
    auto node = _stack[i];
    node->addTimeAndMemory(now, now_memory);
    node->setStartTimeAndMemory(now, now_memory);
  }

  // Zero out the entries
  for (auto & section_time_it : _section_time)
  {
    auto & section_time = section_time_it.second;

    section_time._num_calls = 0;
    section_time._self = 0.;
    section_time._children = 0.;
    section_time._total = 0.;
    section_time._self_memory = 0;
    section_time._children_memory = 0;
    section_time._total_memory = 0.;
  }

  recursivelyFillTime(_root_node.get());

  // Update vector pointing to section times
  // Note: we are doing this _after_ recursively filling
  // because new entries may have been created
  _section_time_ptrs.resize(_id_to_section_info.size());

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

  auto self_memory = current_node->selfMemory();
  auto children_memory = current_node->childrenMemory();
  auto total_memory = current_node->totalMemory();

  // RHS insertion on purpose
  auto & section_time = _section_time[_id_to_section_info[id]._name];

  section_time._self += self;
  section_time._children += children;
  section_time._total += total;
  section_time._num_calls += num_calls;

  section_time._self_memory += self_memory;
  section_time._children_memory += children_memory;
  section_time._total_memory += total_memory;

  for (auto & child_it : current_node->children())
    recursivelyFillTime(child_it.second.get());
}

void
PerfGraph::recursivelyPrintGraph(PerfNode * current_node,
                                 FullTable & vtable,
                                 unsigned int level,
                                 unsigned int current_depth)
{
  mooseAssert(_id_to_section_info.find(current_node->id()) != _id_to_section_info.end(),
              "Unable to find section name!");

  auto & name = _id_to_section_info[current_node->id()]._name;

  mooseAssert(_id_to_section_info.find(current_node->id()) != _id_to_section_info.end(),
              "Unable to find level!");
  auto & node_level = _id_to_section_info[current_node->id()]._level;

  if (node_level <= level)
  {
    mooseAssert(!_section_time_ptrs.empty(),
                "updateTiming() must be run before recursivelyPrintGraph!");

    auto section = std::string(current_depth * 2, ' ') + name;

    // The total time of the root node
    auto total_root_time = std::chrono::duration<double>(_root_node->totalTime()).count();

    auto num_calls = current_node->numCalls();
    auto self = std::chrono::duration<double>(current_node->selfTime()).count();
    auto self_avg = self / static_cast<Real>(num_calls);
    auto self_percent = 100. * self / total_root_time;

    auto total = std::chrono::duration<double>(current_node->totalTime()).count();
    auto total_avg = total / static_cast<Real>(num_calls);
    auto total_percent = 100. * total / total_root_time;

    auto self_memory = current_node->selfMemory();
    auto total_memory = current_node->totalMemory();

    vtable.addRow(section,
                  num_calls,
                  self,
                  self_avg,
                  self_percent,
                  self_memory,
                  total,
                  total_avg,
                  total_percent,
                  total_memory);

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

  auto & name = _id_to_section_info[current_node->id()]._name;

  auto section = std::string(current_depth * 2, ' ') + name;

  // The total time of the root node
  auto total_root_time = _section_time_ptrs[_root_node_id]->_total;

  auto num_calls = current_node->numCalls();
  auto self = std::chrono::duration<double>(current_node->selfTime()).count();
  auto self_avg = self / static_cast<Real>(num_calls);
  auto self_percent = 100. * self / total_root_time;

  auto total = std::chrono::duration<double>(current_node->totalTime()).count();
  auto total_avg = total / static_cast<Real>(num_calls);
  auto total_percent = 100. * total / total_root_time;

  auto self_memory = current_node->selfMemory();
  auto total_memory = current_node->totalMemory();

  vtable.addRow(section,
                num_calls,
                self,
                self_avg,
                self_percent,
                self_memory,
                total,
                total_avg,
                total_percent,
                total_memory);

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
                    "Mem(MB)",
                    "Total(s)",
                    "Avg(s)",
                    "%",
                    "Mem(MB)"},
                   10);

  vtable.setColumnFormat({
      VariadicTableColumnFormat::AUTO,    // Section Name
      VariadicTableColumnFormat::AUTO,    // Calls
      VariadicTableColumnFormat::FIXED,   // Self
      VariadicTableColumnFormat::FIXED,   // Avg.
      VariadicTableColumnFormat::PERCENT, // %
      VariadicTableColumnFormat::AUTO,    // Memory
      VariadicTableColumnFormat::FIXED,   // Total
      VariadicTableColumnFormat::FIXED,   // Avg.
      VariadicTableColumnFormat::PERCENT, // %
      VariadicTableColumnFormat::AUTO,    // Memory
  });

  vtable.setColumnPrecision({
      1, // Section Name
      0, // Calls
      3, // Self
      3, // Avg.
      2, // %
      0, // Memory
      3, // Total
      3, // Avg.
      2, // %
      0, // Memory
  });

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
                    "Mem(MB)",
                    "Total(s)",
                    "Avg(s)",
                    "%",
                    "Mem(MB)"},
                   10);

  vtable.setColumnFormat({VariadicTableColumnFormat::AUTO,    // Section Name
                          VariadicTableColumnFormat::AUTO,    // Calls
                          VariadicTableColumnFormat::FIXED,   // Self
                          VariadicTableColumnFormat::FIXED,   // Avg.
                          VariadicTableColumnFormat::PERCENT, // %
                          VariadicTableColumnFormat::AUTO,    // Memory
                          VariadicTableColumnFormat::FIXED,   // Total
                          VariadicTableColumnFormat::FIXED,   // Avg.
                          VariadicTableColumnFormat::PERCENT, // %
                          VariadicTableColumnFormat::AUTO});  // Memory

  vtable.setColumnPrecision({
      1, // Section Name
      0, // Calls
      3, // Self
      3, // Avg.
      2, // %
      0, // Memory
      3, // Total
      3, // Avg.
      2, // %
      0, // Memory
  });

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

  HeaviestTable vtable({"Section", "Calls", "Self(s)", "Avg.", "%", "Mem(MB)"}, 10);

  vtable.setColumnFormat({VariadicTableColumnFormat::AUTO, // Doesn't matter
                          VariadicTableColumnFormat::AUTO,
                          VariadicTableColumnFormat::FIXED,
                          VariadicTableColumnFormat::FIXED,
                          VariadicTableColumnFormat::PERCENT,
                          VariadicTableColumnFormat::AUTO});

  vtable.setColumnPrecision({1, 1, 3, 3, 2, 1});

  mooseAssert(!_section_time_ptrs.empty(),
              "updateTiming() must be run before printHeaviestSections()!");

  // The total time of the root node
  auto total_root_time = _section_time_ptrs[_root_node_id]->_total;

  // Now print out the largest ones
  for (unsigned int i = 0; i < num_sections; i++)
  {
    auto id = sorted[i];

    if (!_section_time_ptrs[id])
      continue;

    vtable.addRow(_id_to_section_info[id]._name,
                  _section_time_ptrs[id]->_num_calls,
                  _section_time_ptrs[id]->_total_memory,
                  _section_time_ptrs[id]->_self,
                  _section_time_ptrs[id]->_self /
                      static_cast<Real>(_section_time_ptrs[id]->_num_calls),
                  100 * _section_time_ptrs[id]->_self / total_root_time);
  }

  vtable.print(console);
}
