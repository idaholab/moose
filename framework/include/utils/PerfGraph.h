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
#include "ConsoleStream.h"

// System Includes
#include <array>

// Forward Declarations
class PerfGuard;

template <class... Ts>
class VariadicTable;

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
  void print(const ConsoleStream & console, unsigned int level);

  /**
   * Print out the heaviest branch through the tree
   *
   * @param console The output stream to output to
   */
  void printHeaviestBranch(const ConsoleStream & console);

  /**
   * Print out the heaviest sections that were timed
   *
   * @param console The output stream to output to
   */
  void printHeaviestSections(const ConsoleStream & console, const unsigned int num_sections);

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
  void recursivelyPrintGraph(PerfNode * current_node,
                             VariadicTable<std::string, Real, Real, Real> & vtable,
                             unsigned int level,
                             unsigned int current_depth = 0);

  /**
   * Helper for printing out the trace that has taken the most time
   *
   * @param current_node The node to be working on right now
   * @param console Where to print to
   * @param current_depth - Used in the recursion
   */
  void recursivelyPrintHeaviestGraph(PerfNode * current_node,
                                     VariadicTable<std::string, Real, Real, Real> & vtable,
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
  void printHeaviestSections(const ConsoleStream & console);

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

#endif
