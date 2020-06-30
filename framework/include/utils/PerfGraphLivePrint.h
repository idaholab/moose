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

class PerfGraphLivePrint : protected ConsoleStreamInterface
{
public:
  PerfGraphLivePrint(PerfGraph & perf_graph, MooseApp & app);

  /// Start printing
  void start();

protected:
  // Print the live message
  void printLiveMessage(PerfGraph::SectionIncrement & section_increment);

  // Print the stats
  void printStats(PerfGraph::SectionIncrement & section_increment_start, PerfGraph::SectionIncrement & section_increment_finish);

  // Print everything in the stack
  void printStack();

  // Print everything underneath the current top of the stack
  void printStackUpToLast();

  // What to do if we're still in the same spot
  void inSamePlace();

  // What to do if there are new things in the execution list
  void iterateThroughExecutionList();

  const unsigned int WRAP_LENGTH = 90;

  PerfGraph & _perf_graph;

  std::array<PerfGraph::SectionIncrement, MAX_EXECUTION_LIST_SIZE> & _execution_list;

  std::future<bool> _done_future;

  std::map<PerfID, PerfGraph::SectionInfo> & _id_to_section_info;

  /// This is one beyond the last thing on the stack
  unsigned int _stack_level;

  /// The current stack for what the print thread has seen
  std::array<PerfGraph::SectionIncrement *, MAX_STACK_SIZE> _print_thread_stack;

  unsigned int _current_execution_list_end;

  unsigned int _current_execution_list_last;

  unsigned int _last_execution_list_end;

  PerfGraph::SectionIncrement * _last_printed_increment;
};
