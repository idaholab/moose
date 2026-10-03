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
 * Lets every node of a boundary slide in the plane of its support but not through it, which is the
 * inclined roller of Nastran and Abaqus.
 *
 * The condition at a node is u . n = 0, and it is applied as one degree-of-freedom constraint row
 * per node rather than as a penalty term, so it is exact, adds no stiffness, and condenses out of
 * the stiffness and mass matrices alike. The row makes one displacement component depend on the
 * others, which is the shape of a hanging node row.
 *
 * A node where several supports meet carries one row per support, and those rows are eliminated
 * against one another so that no component that is dependent in one of them appears in another.
 * libMesh does not reduce its constraints to independent degrees of freedom until after every
 * constraint object has spoken, so rows left mutually recursive satisfy none of their planes; the
 * elimination is what makes all of them hold at once. It reaches across objects, through the rows
 * already in the DofMap, so the supports may be listed in one object or in one object each.
 *
 * The normals are built once, from the undisplaced mesh, so this is the small deformation,
 * flat-or-locally-flat support. A support whose normal rotates as the body slides over a curved
 * surface needs PenaltyInclinedNoDisplacementBC or a mortar constraint instead.
 */
class InclinedNoDisplacementConstraint : public MultiPointConstraint
{
public:
  static InputParameters validParams();

  InclinedNoDisplacementConstraint(const InputParameters & parameters);

  virtual void addConstraintRows(libMesh::DofMap & dof_map) const override;

  virtual const MooseVariableFieldBase & variable() const override { return *_displacements[0]; }

protected:
  /**
   * Fill _nodal_normals with the unit, area weighted average of the normals of the boundary faces
   * of each support meeting at each of its nodes, taken from the undisplaced mesh.
   *
   * The averaging is per support: the faces of one support that meet at a node are averaged into
   * a single normal, which leaves the node free to slide along the edge they share, but two
   * distinct supports meeting at a node keep their two normals and so their two planes.
   *
   * This is collective: each rank accumulates only the faces of the elements it owns and the
   * partial sums are gathered, so every rank ends up with the same normal at every node and
   * therefore builds the same row.
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

  /// The unit normal of each support at each of its nodes, keyed by boundary id and then by node
  /// id. Every rank holds the normal of every one of those nodes
  std::map<BoundaryID, std::map<dof_id_type, RealVectorValue>> _nodal_normals;
};
