//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PerfGraph.h"

namespace moose
{
namespace internal
{
class PerfGraphRegistry;
}
}

/**
 * This is effectively a functor that runs on a separate thread and
 * watches the state of the call stack to see if things need to be
 * printed about what the application is doing.
 */
class PerfGraphLivePrint : protected ConsoleStreamInterface
{
public:
  PerfGraphLivePrint(PerfGraph & perf_graph, MooseApp & app);

  /// Start printing
  void start();

private:
  /**
   * Print the live message
   */
  void printLiveMessage(PerfGraph::SectionIncrement & section_increment);

  /**
   * Print the stats
   */
  void printStats(PerfGraph::SectionIncrement & section_increment_start,
                  PerfGraph::SectionIncrement & section_increment_finish);

  /**
   * Print everything underneath the current top of the stack
   */
  void printStackUpToLast();

  /**
   * What to do if we're still in the same spot
   */
  void inSamePlace();

  /**
   * What to do if there are new things in the execution list
   */
  void iterateThroughExecutionList();

  /// Number of columns before wrapping
  const unsigned int WRAP_LENGTH = 90;

  /// The app performing this print
  const MooseApp & _app;

  /// Reference to the PerfGraph to work with
  PerfGraph & _perf_graph;

  /// Reference to the PerfGraphRegistry for convenience
  const moose::internal::PerfGraphRegistry & _perf_graph_registry;

  /// Convenience reference to the execution_list within the PerfGraph
  std::array<PerfGraph::SectionIncrement, MAX_EXECUTION_LIST_SIZE> & _execution_list;

  /**
   * True when we stop printing. NOTE: Even though only one thread is supposed to
   * touch this variable, we make it atomic to avoid potential memory leak errors
   * when testing with libtorch. Doing this is not expected to influence the performance
   * considerably.
   */
  std::atomic<bool> _currently_destructing;

  /// Limit (in seconds) before printing
  std::atomic<Real> & _time_limit;

  /// Limit (in MB)
  std::atomic<unsigned int> & _mem_limit;

  /// This is one beyond the last thing on the stack
  unsigned int _stack_level;

  /// The current stack for what the print thread has seen
  std::array<PerfGraph::SectionIncrement, MOOSE_MAX_STACK_SIZE> _print_thread_stack;

  /// The end of the execution list
  /// This is (safely) copied from PerfGraph so that it is consistent for an
  /// entire iteration
  unsigned int _current_execution_list_end;

  /// The actual last entry in the list
  /// This is useful because it is a circular queue - so this is not just
  /// end - 1 (although, often it will be)
  unsigned int _current_execution_list_last;

  /// Where the end of the execution list was during the last call
  /// If this == the current_end... then nothing has happened
  unsigned int _last_execution_list_end;

  /// Which increment was last printed
  PerfGraph::SectionIncrement * _last_printed_increment;

  /// The output count from the console the last time we printed
  unsigned long long int _last_num_printed;

  /// The current output count from the console
  unsigned long long int _console_num_printed;

  /// Whether or not the top thing on the stack is set to print dots
  bool _stack_top_print_dots;
};
