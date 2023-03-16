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

// System Includes
#include <chrono>
#include <memory>

PerfGraph::PerfGraph(const std::string & root_name,
                     MooseApp & app,
                     const bool live_all,
                     const bool perf_graph_live)
  : ConsoleStreamInterface(app),
    _moose_app(app),
    _live_print_all(live_all),
    _disable_live_print(!perf_graph_live),
    _perf_graph_registry(moose::internal::getPerfGraphRegistry()),
    _pid(app.comm().rank()),
    _root_name(root_name),
    _root_node_id(_perf_graph_registry.registerSection(root_name, 0)),
    _root_node(std::make_unique<PerfNode>(_root_node_id)),
    _current_position(-1),
    _stack(),
    _execution_list_begin(0),
    _execution_list_end(0),
    _active(true),
    _live_print_active(true),
    _destructing(false),
    _live_print_time_limit(5.0),
    _live_print_mem_limit(100),
    _live_print(std::make_unique<PerfGraphLivePrint>(*this, app))
{
  if (_pid == 0 && !_disable_live_print)
  {
    // Start the printing thread
    _print_thread = std::thread([this] { this->_live_print->start(); });
  }

  push(_root_node_id);
}

PerfGraph::~PerfGraph() { disableLivePrint(); }

void
PerfGraph::disableLivePrint()
{
  if (_pid == 0 && !_disable_live_print)
  {
    {
      // Unlike using atomics for execution_thread_end
      // here we actually lock to ensure that either the print thread
      // immediately sees that we are destructing or is immediately
      // notified with the below notification.  Without doing this
      // it would be possible (but unlikely) for the print thread to
      // hang for 1 second at the end of execution (which would not be
      // good anytime you are running lots of fast calculations back-to-back
      // like during testing or stochastic sampling).
      std::lock_guard<std::mutex> lock(_destructing_mutex);
      _destructing = true;
    }

    _finished_section.notify_one();

    _print_thread.join();

    _disable_live_print = true;
  }
}

Real
PerfGraph::sectionData(const DataType type,
                       const std::string & section_name,
                       const bool must_exist /* = true */)
{
  update();

  const auto section_it =
      _cumulative_section_info.find(section_name == "Root" ? _root_name : section_name);

  if (section_it == _cumulative_section_info.end())
  {
    if (!must_exist ||                                   // isn't required to exist
        _perf_graph_registry.sectionExists(section_name) // or, is required to exist and it does
    )
      return 0;

    mooseError("Unknown PerfGraph section name \"",
               section_name,
               "\" in PerfGraph::sectionData().\nIf you are attempting to retrieve the root use "
               "\"Root\".");
  }

  const CumulativeSectionInfo & section_info = section_it->second;

  if (type == CALLS)
    return section_info._num_calls;

  const auto app_time = _cumulative_section_info_ptrs[_root_node_id]->_total;

  switch (type)
  {
    case SELF:
      return section_info._self;
    case CHILDREN:
      return section_info._children;
    case TOTAL:
      return section_info._total;
    case SELF_AVG:
      return section_info._self / static_cast<Real>(section_info._num_calls);
    case CHILDREN_AVG:
      return section_info._children / static_cast<Real>(section_info._num_calls);
    case TOTAL_AVG:
      return section_info._total / static_cast<Real>(section_info._num_calls);
    case SELF_PERCENT:
      return 100. * (section_info._self / app_time);
    case CHILDREN_PERCENT:
      return 100. * (section_info._children / app_time);
    case TOTAL_PERCENT:
      return 100. * (section_info._total / app_time);
    case SELF_MEMORY:
      return section_info._self_memory;
    case CHILDREN_MEMORY:
      return section_info._children_memory;
    case TOTAL_MEMORY:
      return section_info._total_memory;
    default:
      ::mooseError("Unknown DataType");
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
  section_increment._beginning_num_printed = _console.numPrinted();

  // A note about this next section of code:
  // It is only EVER run on the main thread - and therefore there can be
  // no race conditions.  All that is important here is that the print
  // thread always sees a consistent value for _execution_list_end
  auto next_execution_list_end = _execution_list_end + 1;

  // Are we at the end of our circular buffer?
  if (next_execution_list_end >= MAX_EXECUTION_LIST_SIZE)
    next_execution_list_end = 0;

  // This "release" will synchronize the above memory changes with the
  // "acquire" in the printing thread
  // All of the above memory operations will be seen by the
  // printing thread before the printing thread sees this new value
  _execution_list_end.store(next_execution_list_end, std::memory_order_release);
}

void
PerfGraph::push(const PerfID id)
{
  if (!_active && !_live_print_active)
    return;

  PerfNode * new_node = nullptr;

  if (id == _root_node_id)
    new_node = _root_node.get();
  else
    new_node = _stack[_current_position]->getChild(id);

  MemoryUtils::Stats stats;
  auto memory_success = MemoryUtils::getMemoryStats(stats);

  long int start_memory = 0;

  if (memory_success)
    start_memory =
        MemoryUtils::convertBytes(stats._physical_memory, MemoryUtils::MemUnits::Megabytes);
  else if (_current_position !=
           -1) // If we weren't able to get the memory stats, let's just use the parent's
    start_memory = _stack[_current_position]->startMemory();

  // Set the start time
  auto current_time = std::chrono::steady_clock::now();

  new_node->setStartTimeAndMemory(current_time, start_memory);

  // Increment the number of calls
  new_node->incrementNumCalls();

  _current_position++;

  if (_current_position >= MAX_STACK_SIZE)
    mooseError("PerfGraph is out of stack space!");

  _stack[_current_position] = new_node;

  // Add this to the execution list unless the message is empty - but pre-emted by live_print_all
  if ((_live_print_active || _live_print_all) && (_pid == 0 && !_disable_live_print) &&
      (!_perf_graph_registry.readSectionInfo(id)._live_message.empty() || _live_print_all))
    addToExecutionList(id, IncrementState::STARTED, current_time, start_memory);
}

void
PerfGraph::pop()
{
  if (!_active && !_live_print_active)
    return;

  auto current_time = std::chrono::steady_clock::now();

  auto & current_node = _stack[_current_position];

  MemoryUtils::Stats stats;
  auto memory_success = MemoryUtils::getMemoryStats(stats);

  long int current_memory = 0;

  if (memory_success)
    current_memory =
        MemoryUtils::convertBytes(stats._physical_memory, MemoryUtils::MemUnits::Megabytes);
  else if (_current_position !=
           -1) // If we weren't able to get the memory stats, let's just use the start memory
    current_memory = _stack[_current_position]->startMemory();

  current_node->addTimeAndMemory(current_time, current_memory);

  _current_position--;

  // Add this to the exection list
  if ((_live_print_active || _live_print_all) && (_pid == 0 && !_disable_live_print) &&
      (!_perf_graph_registry.readSectionInfo(current_node->id())._live_message.empty() ||
       _live_print_all))
  {
    addToExecutionList(current_node->id(), IncrementState::FINISHED, current_time, current_memory);

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
PerfGraph::update()
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
  for (auto & section_time_it : _cumulative_section_info)
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

  recursivelyUpdate(*_root_node);

  // Update vector pointing to section times
  // Note: we are doing this _after_ recursively filling
  // because new entries may have been created
  _cumulative_section_info_ptrs.resize(_perf_graph_registry.numSections());

  for (auto & section_time_it : _cumulative_section_info)
  {
    auto id = _perf_graph_registry.sectionID(section_time_it.first);

    _cumulative_section_info_ptrs[id] = &section_time_it.second;
  }
}

void
PerfGraph::recursivelyUpdate(const PerfNode & current_node)
{
  const auto & section_name = _perf_graph_registry.readSectionInfo(current_node.id())._name;

  // RHS insertion on purpose
  auto & section_time = _cumulative_section_info[section_name];

  section_time._self += current_node.selfTimeSec();
  section_time._children += current_node.childrenTimeSec();
  section_time._total += current_node.totalTimeSec();
  section_time._num_calls += current_node.numCalls();

  section_time._self_memory += current_node.selfMemory();
  section_time._children_memory += current_node.childrenMemory();
  section_time._total_memory += current_node.totalMemory();

  for (auto & child_it : current_node.children())
    recursivelyUpdate(*child_it.second);
}

PerfGraph::FullTable
PerfGraph::treeTable(const unsigned int level, const bool heaviest /* = false */)
{
  update();

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

  auto act = [this, &vtable](const PerfNode & node,
                             const moose::internal::PerfGraphSectionInfo & section_info,
                             const unsigned int depth)
  {
    vtable.addRow(std::string(depth * 2, ' ') + section_info._name,        // Section Name
                  node.numCalls(),                                         // Calls
                  node.selfTimeSec(),                                      // Self
                  node.selfTimeAvg(),                                      // Avg.
                  100. * node.selfTimeSec() / _root_node->totalTimeSec(),  // %
                  node.selfMemory(),                                       // Memory
                  node.totalTimeSec(),                                     // Total
                  node.totalTimeAvg(),                                     // Avg.
                  100. * node.totalTimeSec() / _root_node->totalTimeSec(), // %
                  node.totalMemory());                                     // Memory
  };
  treeRecurse(act, level, heaviest);

  return vtable;
}

void
PerfGraph::print(const ConsoleStream & console, unsigned int level)
{
  console << "\nPerformance Graph:\n";
  treeTable(level).print(console);
}

void
PerfGraph::printHeaviestBranch(const ConsoleStream & console)
{
  console << "\nHeaviest Branch:\n";
  treeTable(MAX_STACK_SIZE, /* heaviest = */ true).print(console);
}

void
PerfGraph::printHeaviestSections(const ConsoleStream & console, const unsigned int num_sections)
{
  update();

  console << "\nHeaviest Sections:\n";

  // Indirect Sort The Self Time
  std::vector<size_t> sorted;
  Moose::indirectSort(_cumulative_section_info_ptrs.begin(),
                      _cumulative_section_info_ptrs.end(),
                      sorted,
                      [](CumulativeSectionInfo * lhs, CumulativeSectionInfo * rhs)
                      {
                        if (lhs && rhs)
                          return lhs->_self > rhs->_self;

                        // If the LHS exists - it's definitely bigger than a non-existant RHS
                        if (lhs)
                          return true;

                        // Both don't exist - so it doesn't matter how we sort them
                        return false;
                      });

  HeaviestTable vtable({"Section", "Calls", "Self(s)", "Avg.", "%", "Mem(MB)"}, 10);

  vtable.setColumnFormat({VariadicTableColumnFormat::AUTO,    // Section; doesn't matter
                          VariadicTableColumnFormat::AUTO,    // Calls
                          VariadicTableColumnFormat::FIXED,   // Time
                          VariadicTableColumnFormat::FIXED,   // Avg.
                          VariadicTableColumnFormat::PERCENT, // Percent
                          VariadicTableColumnFormat::AUTO}    // Memory
  );

  vtable.setColumnPrecision({
      1, // Section
      1, // Calls
      3, // Time
      3, // Avg.
      2, // Percent
      1  // Memory
  });

  mooseAssert(!_cumulative_section_info_ptrs.empty(),
              "update() must be run before printHeaviestSections()!");

  // The total time of the root node
  auto total_root_time = _cumulative_section_info_ptrs[_root_node_id]->_total;

  // Now print out the largest ones
  for (unsigned int i = 0; i < num_sections; i++)
  {
    auto id = sorted[i];

    if (!_cumulative_section_info_ptrs[id])
      continue;

    const auto & entry = *_cumulative_section_info_ptrs[id];
    vtable.addRow(_perf_graph_registry.sectionInfo(id)._name,        // Section
                  entry._num_calls,                                  // Calls
                  entry._self,                                       // Time
                  entry._self / static_cast<Real>(entry._num_calls), // Avg.
                  100. * entry._self / total_root_time,              // Percent
                  entry._self_memory);                               // Memory
  }

  vtable.print(console);
}

void
dataStore(std::ostream & stream, PerfGraph & perf_graph, void *)
{
  // We need to store the registry id -> section info map so that we can add
  // registered sections that may not be added yet during recover
  dataStore(stream, perf_graph._perf_graph_registry._id_to_item, nullptr);

  // Update before serializing the nodes so that the time/memory/calls are correct
  perf_graph.update();

  // Recursively serialize all of the nodes
  dataStore(stream, perf_graph._root_node, nullptr);
}

void
dataLoad(std::istream & stream, PerfGraph & perf_graph, void *)
{
  // Load in all of the recovered sections and register those that do not exist yet
  std::vector<moose::internal::PerfGraphSectionInfo> recovered_section_info;
  dataLoad(stream, recovered_section_info, nullptr);
  for (const auto & info : recovered_section_info)
  {
    if (info._live_message.size())
      perf_graph._perf_graph_registry.registerSection(
          info._name, info._level, info._live_message, info._print_dots);
    else
      perf_graph._perf_graph_registry.registerSection(info._name, info._level);
  }

  // Update the current node time/memory/calls before loading the nodes as the load
  // will append information to current nodes that exist
  perf_graph.update();

  // Recursively load all of the nodes; this will append information to matching nodes
  // and will create new nodes for section paths that do not exist
  dataLoad(stream, perf_graph._root_node, &perf_graph);
}
