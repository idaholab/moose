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
#include "PerfGuard.h"

// System Includes
#include <array>

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
  PerfID registerSection(std::string section_name);

  /**
   * Add a Node onto the end of the end of the current callstack
   */
  void push(PerfID id);

  /**
   * Remove a Node from the end of the current scope
   *
   * @duration The amount of time the Node took
   */
  void pop(std::chrono::steady_clock::duration duration);

  /**
   * Print the tree out
   */
  void print();

  /**
   * Grab the name of a section
   */
  std::string & sectionName(PerfID id);

  /**
   * Whether or not timing is active
   *
   * When not active no timing information will be kept
   */
  bool active() { return _active; }

  /**
   * Turn on or off timing
   */
  void setActive(bool active) { _active = active; }

protected:
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
};

#endif
