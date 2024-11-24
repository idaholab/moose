//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGSurface.h"

namespace CSG
{

/**
 * CSGIntersection creates an internal representation of an intersection of two or more
 * CSGHalfspace objects
 */
class CSGIntersection
{
public:
  /**
   * Default constructor
   */
  CSGIntersection();

  CSGIntersection(std::vector<CSGHalfspace> & nodes);

  /**
   * Destructor
   */
  virtual ~CSGIntersection() = default;

  void addNode(const CSGHalfspace & halfspace) { _nodes.push_back(halfspace); }

  const std::vector<CSGHalfspace> & getNodes() const { return _nodes; }

  std::string toString() const;

protected:
  /// Nodes that consist of intersection
  /// For now each node must be of type CSGHalfspace, but this can be generalized
  /// in the future.
  std::vector<CSGHalfspace> _nodes;
};
} // namespace CSG
