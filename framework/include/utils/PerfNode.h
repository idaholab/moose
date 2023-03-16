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
#include "MemoryUtils.h"

#include <chrono>
#include <map>

class PerfNode
{
public:
  /**
   * Create a PerfNode with the given ID
   */
  PerfNode(const PerfID id) : _id(id), _total_time(0), _num_calls(0), _total_memory(0) {}

  /**
   * Get the ID of this Node
   */
  PerfID id() const { return _id; }

  /**
   * Set the current start time
   */
  void setStartTimeAndMemory(const std::chrono::time_point<std::chrono::steady_clock> time,
                             const long int memory)
  {
    _start_time = time;
    _start_memory = memory;
  }

  /**
   * Get the currnet start time
   * Only makes sense if this node is running.
   */
  const std::chrono::time_point<std::chrono::steady_clock> & startTime() const
  {
    return _start_time;
  }

  /**
   * Get the current start memory
   * Only makes sense if this node is running.
   */
  const long unsigned int & startMemory() const { return _start_memory; }

  /**
   * Add some time into this Node by taking the difference with the time passed in
   */
  void addTimeAndMemory(const std::chrono::time_point<std::chrono::steady_clock> time,
                        const long int memory)
  {
    _total_time += time - _start_time;
    _total_memory += memory - _start_memory;
  }

  /**
   * Increments the number of calls
   */
  void incrementNumCalls() { _num_calls++; }

  /**
   * Add some time into this Node
   */
  void addTimeAndMemory(const std::chrono::steady_clock::duration time) { _total_time += time; }

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
      child_node = std::make_unique<PerfNode>(id);

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
   * Get the time this node took in seconds
   */
  Real selfTimeSec() const { return std::chrono::duration<Real>(selfTime()).count(); }
  /**
   * The average time this node took in seconds
   */
  Real selfTimeAvg() const { return selfTimeSec() / static_cast<Real>(numCalls()); }

  /**
   * The time this Node plus all of it's children took
   */
  std::chrono::steady_clock::duration totalTime() const;
  /**
   * The time this Node plus all of its children took in seconds
   */
  Real totalTimeSec() const { return std::chrono::duration<Real>(totalTime()).count(); }
  /**
   * The average time this Node plus all of its children took in seconds
   */
  Real totalTimeAvg() const { return totalTimeSec() / static_cast<Real>(numCalls()); }

  /**
   * Get the time this nodes children took
   */
  std::chrono::steady_clock::duration childrenTime() const;
  /**
   * The time this node's children took in seconds
   */
  Real childrenTimeSec() const { return std::chrono::duration<Real>(childrenTime()).count(); }

  /**
   * Get the number of times this node was called
   */
  unsigned long int numCalls() const { return _num_calls; }

  /**
   * Get the amount of memory added by this node
   */
  long int selfMemory() const;

  /**
   * Get the amount of memory added by this node
   */
  long int childrenMemory() const;

  /**
   * Get the amount of memory added by this node
   */
  long int totalMemory() const { return _total_memory; }

private:
  /// The unique ID for the section this Node corresponds to
  const PerfID _id;

  /// The current start_time for this node (if it's on the stack)
  std::chrono::time_point<std::chrono::steady_clock> _start_time;

  /// The total elapsed time for this node
  std::chrono::steady_clock::duration _total_time;

  /// The starting memory for this node
  long unsigned int _start_memory;

  /// Number of times this node has been called
  long unsigned int _num_calls;

  /// The total memory added while this node is active
  long unsigned int _total_memory;

  /// Timers that are directly underneath this node
  std::map<PerfID, std::unique_ptr<PerfNode>> _children;

  friend void dataStore(std::ostream &, const std::unique_ptr<PerfNode> &, void *);
  friend void dataLoad(std::istream &, const std::unique_ptr<PerfNode> &, void *);
};

void dataStore(std::ostream & stream, const std::unique_ptr<PerfNode> & node, void * context);
void dataLoad(std::istream & stream, const std::unique_ptr<PerfNode> & node, void * context);
