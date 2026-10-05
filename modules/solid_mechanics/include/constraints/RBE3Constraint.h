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
 * weighted least-squares rigid-body fit of the displacements of a set of independent nodes, which
 * is the kinematics of a Nastran RBE3 element.
 *
 * With the weight, the position and the displacement of independent node i written as \f$ w_i \f$,
 * \f$ \mathbf{x}_i \f$ and \f$ \mathbf{u}_i \f$, the total weight as \f$ W = \sum_i w_i \f$, the
 * weighted centroid as \f$ \mathbf{x}_c = \sum_i w_i \mathbf{x}_i / W \f$, and with
 * \f$ \mathbf{r}_i = \mathbf{x}_i - \mathbf{x}_c \f$, \f$ \mathbf{d} = \mathbf{x}_{ref} -
 * \mathbf{x}_c \f$ and the weighted second-moment tensor \f$ J = \sum_i w_i (|\mathbf{r}_i|^2 I -
 * \mathbf{r}_i \mathbf{r}_i^T) \f$, the motion that minimizes \f$ \sum_i w_i |\mathbf{u}_i -
 * (\bar{\mathbf{u}} + \boldsymbol{\theta} \times \mathbf{r}_i)|^2 \f$ gives
 *
 * \f$ \bar{\mathbf{u}} = \frac{1}{W} \sum_i w_i \mathbf{u}_i \f$,
 * \f$ \boldsymbol{\theta} = J^{-1} \sum_i w_i (\mathbf{r}_i \times \mathbf{u}_i) \f$,
 * \f$ \mathbf{u}_{ref} = \bar{\mathbf{u}} + \boldsymbol{\theta} \times \mathbf{d} \f$ and
 * \f$ \boldsymbol{\theta}_{ref} = \boldsymbol{\theta} \f$.
 *
 * The rows hold the derivatives of this motion, which are constant because the coefficients are
 * built from the undisplaced mesh. Since the rows are applied to every matrix and vector as
 * \f$ C^T (\cdot) C \f$, a force on the reference node is distributed over the independent nodes as
 * \f$ \mathbf{F}_i = (w_i / W) \mathbf{F} + w_i (J^{-1} (\mathbf{d} \times \mathbf{F})) \times
 * \mathbf{r}_i \f$ and no stiffness is added. Only the translations of the independent nodes are
 * used, which are the default components 1 through 3 of a Nastran RBE3 element.
 */
class RBE3Constraint : public MultiPointConstraint
{
public:
  static InputParameters validParams();

  RBE3Constraint(const InputParameters & parameters);

  virtual const MooseVariableBase & variable() const override;

  virtual void addConstraintRows(libMesh::DofMap & dof_map) const override;

protected:
  /// The node set holding the single dependent reference node
  const BoundaryName _reference_boundary;

  /// The node sets whose nodes form the independent set
  const std::vector<BoundaryName> _independent_boundaries;

  /// The weight of each entry of _independent_boundaries, shared by every node of that node set
  std::vector<Real> _weights;

  /// The three displacement variables, which are constrained at the reference node and are the
  /// only variables of the independent nodes this constraint uses
  std::vector<const MooseVariableFieldBase *> _displacement_vars;

  /// The three rotation variables of the reference node, empty when 'rotations' is not given
  std::vector<const MooseVariableFieldBase *> _rotation_vars;

  /// The system numbers of the displacement variables, which are the degrees of freedom gathered
  /// on the independent nodes
  std::vector<unsigned int> _displacement_var_numbers;

  /// The variables constrained at the reference node, the displacements followed by the rotations,
  /// which is the order in which the degrees of freedom of the reference node are gathered
  std::vector<const MooseVariableFieldBase *> _reference_vars;
};
