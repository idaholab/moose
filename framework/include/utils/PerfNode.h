//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
/**
 * A node in the PerfGraph.  Stores the timing for a particular section of code.
 *
 * The design here is that _children are automatically added and kept track of
 * within the Node - and only raw pointers are handed back out.
 */


#include "MooseTypes.h"

#include <chrono>
#include <map>

class PerfNode
{
public:
  /**
   * Create a PerfNode with the given ID
   */
  PerfNode(const PerfID id) : _id(id), _total_time(0), _num_calls(0) {}

  /**
   * Get the ID of this Node
   */
  PerfID id() const { return _id; }

  /**
   * Set the current start time
   */
  void setStartTime(const std::chrono::time_point<std::chrono::steady_clock> time)
  {
    _start_time = time;
  }

  /**
   * Add some time into this Node by taking the difference with the time passed in
   */
  void addTime(const std::chrono::time_point<std::chrono::steady_clock> time)
  {
    _total_time += time - _start_time;
  }

  /**
   * Increments the number of calls
   */
  void incrementNumCalls() { _num_calls++; }

  /**
   * Add some time into this Node
   */
  void addTime(const std::chrono::steady_clock::duration time) { _total_time += time; }

  /**
   * Get a child node with the unique id given
   *
   * Note: this will automatically create the Node internally if it needs to.
   *
   * Implemented in header to allow for more optimization
   *
   * @param id The unique ID of the child node
   * @return The pointer to the child node
   */
  PerfNode * getChild(const PerfID id)
  {
    // RHS insertion on purpose
    auto & child_node = _children[id];

    if (!child_node)
      child_node.reset(new PerfNode(id));

    return child_node.get();
  }

  /**
   * Get the children
   */
  const std::map<PerfID, std::unique_ptr<PerfNode>> & children() const { return _children; }

  /**
   * Get the time this node took
   */
  std::chrono::steady_clock::duration selfTime() const;

  /**
   * The time this Node plus all of it's children took
   */
  std::chrono::steady_clock::duration totalTime() const;

  /**
   * Get the time this nodes children took
   */
  std::chrono::steady_clock::duration childrenTime() const;

  /**
   * Get the number of times this node was called
   */
  unsigned long int numCalls() { return _num_calls; }

protected:
  /// The unique ID for the section this Node corresponds to
  PerfID _id;

  /// The current start_time for this node (if it's on the stack)
  std::chrono::time_point<std::chrono::steady_clock> _start_time;

  /// The total elapsed time for this node
  std::chrono::steady_clock::duration _total_time;

  /// Number of times this node has been called
  long unsigned int _num_calls;

  /// Timers that are directly underneath this node
  std::map<PerfID, std::unique_ptr<PerfNode>> _children;
};
