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

#ifndef EQUALVALUEBOUNDARYCONSTRAINT_H
#define EQUALVALUEBOUNDARYCONSTRAINT_H

#include "NodalConstraint.h"

class EqualValueBoundaryConstraint;

template <>
InputParameters validParams<EqualValueBoundaryConstraint>();

class EqualValueBoundaryConstraint : public NodalConstraint
{
public:
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
   * Computes the residual for the current slave node
   */
  virtual Real computeQpResidual(Moose::ConstraintType type) override;

  /**
   * Computes the jacobian for the constraint
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  // Holds the master node id
  unsigned int _master_node_id;
  // Holds the list of slave node ids
  std::vector<unsigned int> _slave_node_ids;
  // Holds the slave node set or side set
  BoundaryName _slave_node_set_id;
  // Penalty if constraint is not satisfied
  Real _penalty;
};

#endif /* EQUALVALUEBOUNDARYCONSTRAINT_H */
