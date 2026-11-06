//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiPointConstraint.h"

#include <map>

/**
 * Inclined roller support: u . n = 0 at every node of a boundary, as one exact constraint per node
 * and support. Where supports meet, their constraints are eliminated against each other, including
 * those of other objects already in the DofMap. Normals come from the undisplaced mesh, so this is
 * a small deformation support.
 */
class InclinedNoDisplacementConstraint : public MultiPointConstraint
{
public:
  static InputParameters validParams();

  InclinedNoDisplacementConstraint(const InputParameters & parameters);

  virtual void addExactConstraints(libMesh::DofMap & dof_map) const override;

  virtual const MooseVariableFieldBase & variable() const override { return *_displacements[0]; }

protected:
  /**
   * Fill _nodal_normals with the area weighted unit normal of each support at each of its nodes.
   * Distinct supports at a node keep separate normals. Every rank must call this.
   */
  void computeNodalNormals();

  /// The node sets or side sets of the support
  const std::vector<BoundaryName> & _boundaries;

  /// The displacement variables, in the x, y[, z] order of the input
  std::vector<const MooseVariableFieldBase *> _displacements;

  /// Whether 'normal' was given, in which case _nodal_normals stays empty
  const bool _has_constant_normal;

  /// The unit support normal of 'normal', when the user gave one
  RealVectorValue _constant_normal;

  /// The unit normal of each support at each of its nodes, keyed by boundary id then node id
  std::map<BoundaryID, std::map<dof_id_type, RealVectorValue>> _nodal_normals;
};
