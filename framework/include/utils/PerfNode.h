//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/**
 * A node in the PerfGraph.  Stores the timing for a particular section of code.
 *
 * The design here is that _children are automatically added and kept track of
 * within the Node - and only raw pointers are handed back out.
 */

#ifndef PERFNODE_H
#define PERFNODE_H

#include "MooseTypes.h"

#include <map>

class PerfNode
{
public:
  /**
   * Create a PerfNode with the given ID
   */
  PerfNode(PerfID id);

  /**
   * Get the ID of this Node
   */
  PerfID id() { return _id; }

  /**
   * Add some time into this Node
   */
  void addTime(std::chrono::steady_clock::duration time) { _total_time += time; }

  /**
   * Get a child node with the unique id given
   *
   * Note: this will automatically create the Node internally if it needs to.
   *
   * @param id The unique ID of the child node
   * @return The pointer to the child node
   */
  PerfNode * getChild(PerfID id);

  /**
   * Get the children
   */
  std::map<PerfID, std::unique_ptr<PerfNode>> & children() { return _children; }

  /**
   * Get the time this node took
   */
  std::chrono::steady_clock::duration selfTime();

  /**
   * The time this Node plus all of it's children took
   */
  std::chrono::steady_clock::duration totalTime();

  /**
   * Get the time this nodes children took
   */
  std::chrono::steady_clock::duration childrenTime();

protected:
  /// The unique ID for the section this Node corresponds to
  PerfID _id;

  /// The total elapsed time for this node
  std::chrono::steady_clock::duration _total_time;

  /// Timers that are directly underneath this node
  std::map<PerfID, std::unique_ptr<PerfNode>> _children;
};

#endif
