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

/**
 * The secondary node variable is programmed as a linear combination of
 * the primary node variables (i.e, secondary_var = a_1*primary_var_1+
 * a_2*primary_var_2+... + a_n*primary_var_n).  The primary nodes ids and
 * corresponding weights are required as input.  The same linear
 * combination applies to all secondary nodes.
 */
class LinearNodalConstraint : public NodalConstraint
{
public:
  static InputParameters validParams();

  LinearNodalConstraint(const InputParameters & parameters);

protected:
  /**
   * Computes the residual for the current secondary node
   */
  virtual Real computeQpResidual(Moose::ConstraintType type) override;

  /**
   * Computes the jacobian for the constraint
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  // Holds the primary node ids
  std::vector<unsigned int> _primary_node_ids;
  // Holds the list of secondary node ids
  std::vector<unsigned int> _secondary_node_ids;
  // Holds the secondary node set or side set
  std::string _secondary_node_set_id;
  // Penalty if constraint is not satisfied
  Real _penalty;
};
