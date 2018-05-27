//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PERFGRAPH_H
#define PERFGRAPH_H

// MOOSE Includes
#include "MooseTypes.h"
#include "PerfNode.h"
#include "IndirectSort.h"

// System Includes
#include <array>

class PerfGuard;

#define MAX_STACK_SIZE 100

/**
 * The PerfGraph will hold the master list of all registered performance segments and
 * the head PerfNode
 */
class PerfGraph
{
public:
  /**
   * Create a new PerfGraph
   */
  PerfGraph();

  /**
   * Destructor
   */
  ~PerfGraph();

  /**
   * Registers a named section of code
   *
   * @return The unique ID to use for that section
   */
  PerfID registerSection(const std::string & section_name, unsigned int level);

  /**
   * Print the tree out
   *
   * @param console The output stream to output to
   * @param level The log level, the higher the number the more output you get
   */
  template <typename StreamType>
  void print(StreamType & console, unsigned int level);

  /**
   * Grab the name of a section
   */
  const std::string & sectionName(const PerfID id) const;

  /**
   * Whether or not timing is active
   *
   * When not active no timing information will be kept
   */
  bool active() const { return _active; }

  /**
   * Turn on or off timing
   */
  void setActive(bool active) { _active = active; }

protected:
  /**
   * Add a Node onto the end of the end of the current callstack
   *
   * Note: only accessible by using PerfGuard!
   */
  void push(const PerfID id);

  /**
   * Remove a Node from the end of the current scope
   *
   * Note: only accessible by using PerfGuard!
   */
  void pop();

  /**
   * Udates the time for all currently running nodes
   */
  void updateCurrentlyRunning();

  /**
   * Helper for printing out the graph
   *
   * @param current_node The node to be working on right now
   * @param console Where to print to
   * @param level The level to print out below (<=)
   * @param current_depth - Used in the recursion
   */
  template <typename StreamType>
  void recursivelyPrintGraph(PerfNode * current_node,
                             StreamType & console,
                             unsigned int level,
                             unsigned int current_depth = 0);

  /**
   * Helper for printing out the trace that has taken the most time
   *
   * @param current_node The node to be working on right now
   * @param console Where to print to
   * @param current_depth - Used in the recursion
   */
  template <typename StreamType>
  void recursivelyPrintHeaviestGraph(PerfNode * current_node,
                                     StreamType & console,
                                     unsigned int current_depth = 0);

  /**
   * Small helper function to fill in a vector with the total self time for each timer ID
   *
   * @param current_node The current node to work on
   * @param section_self_time A vector of num_ids size to sum self time into
   */
  void recursivelyFillTotalSelfTime(PerfNode * current_node, std::vector<Real> & section_self_time);

  /**
   * Helper for printing out the heaviest sections
   *
   * @param console Where to print to
   */
  template <typename StreamType>
  void printHeaviestSections(StreamType & console);

  /// The root node of the graph
  std::unique_ptr<PerfNode> _root_node;

  /// The current node position in the stack
  unsigned int _current_position;

  /// The full callstack.  Currently capped at a depth of 100
  std::array<PerfNode *, MAX_STACK_SIZE> _stack;

  /// Map of section names to IDs
  std::map<std::string, PerfID> _section_name_to_id;

  /// Map of IDs to section names
  std::map<PerfID, std::string> _id_to_section_name;

  /// Map of IDs to level
  std::map<PerfID, unsigned int> _id_to_level;

  /// Whether or not timing is active
  bool _active;

  // Here so PerfGuard is the only thing that can call push/pop
  friend class PerfGuard;
};

template <typename StreamType>
void
PerfGraph::recursivelyPrintGraph(PerfNode * current_node,
                                 StreamType & console,
                                 unsigned int level,
                                 unsigned int current_depth)
{
  auto & name = _id_to_section_name[current_node->id()];
  auto & node_level = _id_to_level[current_node->id()];

  if (node_level <= level)
  {
    console << std::string(current_depth * 2, ' ') << name
            << " self: " << std::chrono::duration<double>(current_node->selfTime()).count()
            << " children: " << std::chrono::duration<double>(current_node->childrenTime()).count()
            << " total: " << std::chrono::duration<double>(current_node->totalTime()).count()
            << "\n";

    current_depth++;
  }

  for (auto & child_it : current_node->children())
    recursivelyPrintGraph(child_it.second.get(), console, level, current_depth);
}

template <typename StreamType>
void
PerfGraph::recursivelyPrintHeaviestGraph(PerfNode * current_node,
                                         StreamType & console,
                                         unsigned int current_depth)
{
  auto & name = _id_to_section_name[current_node->id()];

  console << std::string(current_depth * 2, ' ') << name
          << " self: " << std::chrono::duration<double>(current_node->selfTime()).count()
          << " children: " << std::chrono::duration<double>(current_node->childrenTime()).count()
          << " total: " << std::chrono::duration<double>(current_node->totalTime()).count() << "\n";

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

    recursivelyPrintHeaviestGraph(heaviest_child, console, current_depth);
  }
}

template <typename StreamType>
void
PerfGraph::printHeaviestSections(StreamType & console)
{
  // First - accumulate the time for each section
  std::vector<Real> section_self_time(_section_name_to_id.size(), 0.);
  recursivelyFillTotalSelfTime(_root_node.get(), section_self_time);

  // Now indirect sort them
  std::vector<size_t> sorted;
  Moose::indirectSort(section_self_time.begin(),
                      section_self_time.end(),
                      sorted,
                      [](double lhs, double rhs) { return lhs > rhs; });

  // Now print out the largest ones
  for (unsigned int i = 0; i < 5; i++)
  {
    auto id = sorted[i];

    console << _id_to_section_name[id] << " Total Self Time: " << section_self_time[id] << "\n";
  }
}

template <typename StreamType>
void
PerfGraph::print(StreamType & console, unsigned int level)
{
  updateCurrentlyRunning();

  console << "\nPerformance Graph:\n";
  recursivelyPrintGraph(_root_node.get(), console, level);

  console << "\nHeaviest Branch:\n";
  recursivelyPrintHeaviestGraph(_root_node.get(), console);

  console << "\nHeaviest Sections:\n";
  printHeaviestSections(console);
}

#endif
