//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * Base class for mesh modifiers that snap nodes to a defined geometry either when executed
 * or when mesh adaptivity happens
 */
class MoveNodesToGeometryModifierBase : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MoveNodesToGeometryModifierBase(const InputParameters & parameters);

  virtual void initialize() final;
  virtual void execute() final;
  virtual void finalize() final;

  virtual void meshChanged() final;

protected:
  /// Snap all nodes from the specified block or boundary restriction to the derived-class-defined geometry
  void snapNodes();

  /**
   * Override this method in derived classes to implement a specific geometry.
   * The method takes a writable reference to a node. Set the position of the
   * node to the closest point on the surface of the true geometry.
   */
  virtual void snapNode(Node & node) = 0;

  /// Reference to the current simulation mesh
  MooseMesh & _mesh;

  /// List of boundaries (or node sets) from which nodes will be snapped to a geometry
  const std::vector<BoundaryID> _boundary_ids;

  /// List of blocks (likely lower D blocks) from which nodes will be snapped to a geometry
  const std::vector<SubdomainID> _subdomain_ids;
};
