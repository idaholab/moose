//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalUserObject.h"

/**
 * Finds the nearest node to a given point.  Upon a tie, the node with the smallest id() is
 * used
 */
class NearestNodeNumberUO : public NodalUserObject
{
public:
  static InputParameters validParams();

  NearestNodeNumberUO(const InputParameters & parameters);

  virtual void meshChanged() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

  /// Returns the ID of the nearest node
  dof_id_type getClosestNodeId() const;

  /**
   * Returns a const pointer to the closest node over the entire mesh, if that node is owned by this
   * processor, otherwise returns nullptr.  Also returns nullptr if no execute() has been performed
   * yet.
   */
  const Node * getClosestNode() const;

private:
  /// processor ID of this object
  const processor_id_type _my_pid;

  /// The point
  const Point & _point;

  /// Whether the nearest node on this processor has been found (this is used to reduce unnecessary re-computations)
  bool _node_found;

  /// Minimum distance for nodes on this processor
  Real _min_distance;

  /// Nearest node on this processor
  const Node * _closest_node;

  /// Node number of closest node, over the whole mesh
  dof_id_type _overall_best_id;
};
