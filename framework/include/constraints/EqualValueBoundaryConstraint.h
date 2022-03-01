//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

  // Holds the primary node id
  unsigned int _primary_node_id;
  // Holds the list of secondary node ids
  std::vector<unsigned int> _secondary_node_ids;
  // Holds the secondary node set or side set
  BoundaryName _secondary_node_set_id;
  // Penalty if constraint is not satisfied
  Real _penalty;
};
