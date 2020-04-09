//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE Includes
#include "MooseTypes.h"
#include "PerfNode.h"
#include "IndirectSort.h"
#include "ConsoleStream.h"
#include "MooseError.h"

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
   * For retrieving values
   */
  enum TimeType : char
  {
    SELF,
    CHILDREN,
    TOTAL,
    SELF_AVG,
    CHILDREN_AVG,
    TOTAL_AVG,
    SELF_PERCENT,
    CHILDREN_PERCENT,
    TOTAL_PERCENT
  };

  /**
   * Create a new PerfGraph
   */
  PerfGraph(const std::string & root_name);

  /**
   * Destructor
   */
  ~PerfGraph() = default;

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

  /**
   * Get the number of calls for a section
   */
  unsigned long int getNumCalls(const std::string & section_name);

  /**
   * Get a reference to the time for a section
   */
  Real getTime(const TimeType type, const std::string & section_name);

  /**
   * Get a reference to the self time for a section
   *
   * This reference can be held onto and the value
   * will be updated anytime updateTiming() is called.
   */
  const Real & getSelfTime(const std::string & section_name)
  {
    return _section_time[section_name]._self;
  }

  /**
   * Get a reference to the children time for a section
   *
   * This reference can be held onto and the value
   * will be updated anytime updateTiming() is called.
   */
  const Real & getChildrenTime(const std::string & section_name)
  {
    return _section_time[section_name]._children;
  }

  /**
   * Get a reference to the total time for a section
   *
   * This reference can be held onto and the value
   * will be updated anytime updateTiming() is called.
   */
  const Real & getTotalTime(const std::string & section_name)
  {
    return _section_time[section_name]._total;
  }

  /**
   * Udates the time section_time and time for all currently running nodes
   */
  void updateTiming();

protected:
  typedef VariadicTable<std::string,
                        unsigned long int,
                        Real,
                        Real,
                        Real,
                        Real,
                        Real,
                        Real,
                        Real,
                        Real,
                        Real>
      FullTable;

  typedef VariadicTable<std::string, unsigned long int, Real, Real, Real> HeaviestTable;

  /**
   * Use to hold the time for each section
   *
   * These will be filled by updateTiming()
   */
  struct SectionTime
  {
    Real _self = 0.;
    Real _children = 0.;
    Real _total = 0.;
    unsigned long int _num_calls = 0;
  };

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
   * Helper for printing out the graph
   *
   * @param current_node The node to be working on right now
   * @param console Where to print to
   * @param level The level to print out below (<=)
   * @param current_depth - Used in the recursion
   */
  void recursivelyPrintGraph(PerfNode * current_node,
                             FullTable & vtable,
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
                                     FullTable & vtable,
                                     unsigned int current_depth = 0);

  /**
   * Updates the cumulative self/children/total time
   *
   * Note: requires that self/children/total time are resized and zeroed before calling.
   *
   * @param current_node The current node to work on
   */
  void recursivelyFillTime(PerfNode * current_node);

  /**
   * Helper for printing out the heaviest sections
   *
   * @param console Where to print to
   */
  void printHeaviestSections(const ConsoleStream & console);

  /// The name (handle) of the root node
  static const std::string ROOT_NAME;

  /// The name to display for the root of the graph when printing the entire graph
  const std::string _root_name;

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

  /// The time for each section.  This is updated on updateTiming()
  /// Note that this is _total_ cumulative time across every place
  /// that section is in the graph
  ///
  /// I'm making this a map so that we can give out references to the values
  /// The three values are: self, children
  /// The map is on std::string because we might need to be able to retrieve
  /// timing values in a "late binding" situation _before_ the section
  /// has been registered.
  std::map<std::string, SectionTime> _section_time;

  /// Pointers into _section_time indexed on PerfID
  /// This is here for convenience and speed so we don't need
  /// to iterate over the above map much - and it makes it
  /// easier to sort
  std::vector<SectionTime *> _section_time_ptrs;

  /// Whether or not timing is active
  bool _active;

  // Here so PerfGuard is the only thing that can call push/pop
  friend class PerfGuard;
};
