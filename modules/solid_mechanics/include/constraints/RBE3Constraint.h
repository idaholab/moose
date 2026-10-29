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

class MooseVariableBase;

/**
 * Constrains the displacements, and optionally the rotations, of a reference node to the
 * weighted least-squares rigid-body fit of the displacements of a set of independent nodes. With
 * centroid \f$ \mathbf{x}_c \f$, \f$ \mathbf{r}_i = \mathbf{x}_i - \mathbf{x}_c \f$,
 * \f$ \mathbf{d} = \mathbf{x}_{ref} - \mathbf{x}_c \f$ and
 * \f$ J = \sum_i w_i (|\mathbf{r}_i|^2 I - \mathbf{r}_i \mathbf{r}_i^T) \f$:
 * \f$ \boldsymbol{\theta} = J^{-1} \sum_i w_i (\mathbf{r}_i \times \mathbf{u}_i) \f$ and
 * \f$ \mathbf{u}_{ref} = \sum_i w_i \mathbf{u}_i / W + \boldsymbol{\theta} \times \mathbf{d} \f$.
 * Only the translations of the independent nodes are used.
 */
class RBE3Constraint : public MultiPointConstraint
{
public:
  static InputParameters validParams();

  RBE3Constraint(const InputParameters & parameters);

  virtual const MooseVariableBase & variable() const override;

  virtual void addExactConstraints(libMesh::DofMap & dof_map) const override;

protected:
  /// The node set holding the single dependent reference node
  const BoundaryName _reference_boundary;

  /// The node sets whose nodes form the independent set
  const std::vector<BoundaryName> _independent_boundaries;

  /// The weight of each entry of _independent_boundaries, shared by every node of that node set
  std::vector<Real> _weights;

  /// The three displacement variables
  std::vector<const MooseVariableFieldBase *> _displacement_vars;

  /// The three rotation variables of the reference node, empty when 'rotations' is not given
  std::vector<const MooseVariableFieldBase *> _rotation_vars;

  /// The system numbers of the displacement variables
  std::vector<unsigned int> _displacement_var_numbers;

  /// The variables constrained at the reference node, displacements then rotations
  std::vector<const MooseVariableFieldBase *> _reference_vars;
};
