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
 * Base class for userobjects that snap nodes to a defined geometry when
 * adaptivity happens.
 */
class GeometryBase : public GeneralUserObject
{
public:
  static InputParameters validParams();

  GeometryBase(const InputParameters & parameters);

  virtual void initialize() final;
  virtual void execute() final;
  virtual void finalize() final;

  virtual void meshChanged() final;

protected:
  /**
   * Override this method in derived classes to implement a specific geometry.
   * The method takes a writable reference to a node. Set the position of the
   * node to the closest point on the surface of the true geometry.
   */
  virtual void snapNode(Node & node) = 0;

  /// Reference to the current simulation mesh
  MooseMesh & _mesh;

  /// List of boundaries (or node sets) that will be snapped to a geometry
  const std::vector<BoundaryID> _boundary_ids;
};
