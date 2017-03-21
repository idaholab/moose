/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
