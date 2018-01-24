//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LINEARNODALCONSTRAINT_H
#define LINEARNODALCONSTRAINT_H

#include "NodalConstraint.h"

class LinearNodalConstraint;

template <>
InputParameters validParams<LinearNodalConstraint>();

/**
 * The slave node variable is programmed as a linear combination of
 * the master node variables (i.e, slave_var = a_1*master_var_1+
 * a_2*master_var_2+... + a_n*master_var_n).  The master nodes ids and
 * corresponding weights are required as input.  The same linear
 * combination applies to all slave nodes.
 */
class LinearNodalConstraint : public NodalConstraint
{
public:
  LinearNodalConstraint(const InputParameters & parameters);

protected:
  /**
   * Computes the residual for the current slave node
   */
  virtual Real computeQpResidual(Moose::ConstraintType type) override;

  /**
   * Computes the jacobian for the constraint
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  // Holds the master node ids
  std::vector<unsigned int> _master_node_ids;
  // Holds the list of slave node ids
  std::vector<unsigned int> _slave_node_ids;
  // Holds the slave node set or side set
  std::string _slave_node_set_id;
  // Penalty if constraint is not satisfied
  Real _penalty;
};

#endif /* LINEARNODALCONSTRAINT_H */
