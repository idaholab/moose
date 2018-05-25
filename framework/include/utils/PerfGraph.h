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
  PerfID registerSection(const std::string & section_name);

  /**
   * Print the tree out
   */
  template <typename StreamType>
  void print(StreamType & console);

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
   */
  void push(const PerfID id);

  /**
   * Remove a Node from the end of the current scope
   *
   * @duration The amount of time the Node took
   */
  void pop(const std::chrono::steady_clock::duration duration);

  /**
   * Udates the time for the root node
   */
  void updateRoot();

  /// The root node of the graph
  std::unique_ptr<PerfNode> _root_node;

  /// The start time of the root
  std::chrono::time_point<std::chrono::steady_clock> _root_start;

  /// The current node position in the stack
  unsigned int _current_position;

  /// The full callstack.  Currently capped at a depth of 100
  std::array<PerfNode *, MAX_STACK_SIZE> _stack;

  /// Map of section names to IDs
  std::map<std::string, PerfID> _section_name_to_id;

  /// Map of IDs to section names
  std::map<PerfID, std::string> _id_to_section_name;

  /// Whether or not timing is active
  bool _active;

  // Here so PerfGuard is the only thing that can call push/pop
  friend class PerfGuard;
};

/**
 * Helper function to recursively print out the graph
 */
template <typename StreamType>
void
recursivelyPrintGraph(PerfGraph & graph,
                      PerfNode * current_node,
                      StreamType & console,
                      unsigned int current_depth = 0)
{
  auto & name = graph.sectionName(current_node->id());

  console << std::string(current_depth * 2, ' ') << name
          << " self: " << std::chrono::duration<double>(current_node->selfTime()).count()
          << " children: " << std::chrono::duration<double>(current_node->childrenTime()).count()
          << " total: " << std::chrono::duration<double>(current_node->totalTime()).count() << "\n";

  for (auto & child_it : current_node->children())
    recursivelyPrintGraph(graph, child_it.second.get(), console, current_depth + 1);
}

template <typename StreamType>
void
PerfGraph::print(StreamType & console)
{
  updateRoot();
  recursivelyPrintGraph(*this, _root_node.get(), console);
}

#endif
