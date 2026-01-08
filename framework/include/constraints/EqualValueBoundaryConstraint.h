//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalConstraint.h"

class EqualValueBoundaryConstraint : public NodalConstraint
{
public:
  static InputParameters validParams();

  EqualValueBoundaryConstraint(const InputParameters & parameters);

  /**
   * Called on this object when the mesh changes
   */
  virtual void meshChanged() override;

protected:
  /**
   * Update the sets of nodes with constrained DOFs
   */
  void updateConstrainedNodes();

  /**
   * Computes the residual for the current secondary node
   */
  virtual Real computeQpResidual(Moose::ConstraintType type) override;

  /**
   * Computes the jacobian for the constraint
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  /// Penalty if constraint is not satisfied
  Real _penalty;

private:
  /**
   * Populate the set of secondary nodes from user input
   */
  void populateSecondaryNodes();

  /**
   * Pick the primary node from user input or from the secondary node set
   */
  void pickPrimaryNode();

  /**
   * Ghost elements and nodes connected to the primary node
   */
  void ghostPrimary();

  /**
   * Get the primary node ID by searching for the node with coordinates
   * matching _primary_node_coord on the secondary node set
   */
  dof_id_type getPrimaryNodeIDByCoord() const;
};
