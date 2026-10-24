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

/**
 * Ties every node of a boundary rigidly to a single independent node that carries three
 * translations and three rotations, with small-rotation kinematics on the undisplaced mesh.
 */
class RBE2Constraint : public MultiPointConstraint
{
public:
  static InputParameters validParams();

  RBE2Constraint(const InputParameters & parameters);

  virtual void addExactConstraints(libMesh::DofMap & dof_map) const override;

  virtual const MooseVariableFieldBase & variable() const override { return *_displacements[0]; }

protected:
  /// The node set holding the single independent node the dependent nodes follow
  const BoundaryName & _independent_boundary;

  /// The node set or side set holding the dependent nodes
  const BoundaryName & _dependent_boundary;

  /// Whether the rotations of the dependent nodes are tied to the rotations of the independent node
  const bool _tie_rotations;

  /// The three displacement variables, in the x, y, z order of the input
  std::vector<const MooseVariableFieldBase *> _displacements;

  /// The three rotation variables of the independent node, in the x, y, z order of the input
  std::vector<const MooseVariableFieldBase *> _rotations;

  /// The displacement then rotation variables, the gather order of the independent node
  std::vector<const MooseVariableFieldBase *> _variables;
};
