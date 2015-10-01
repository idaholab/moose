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

template<>
InputParameters validParams<LinearNodalConstraint>();

class LinearNodalConstraint : public NodalConstraint
{
public:
  LinearNodalConstraint(const InputParameters & parameters);
  virtual ~LinearNodalConstraint();

protected:

  /**
   * Computes the residual for the current slave node
   */
  virtual Real computeQpResidual(Moose::ConstraintType type, NumericVector<Number> & residual);

  /**
   * Computes the jacobian for the constraint
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type, SparseMatrix<Number> & jacobian);

  // Holds the master node ids
  std::vector<unsigned int> _master_node_ids;
  // Holds the list of slave node ids
  std::vector<unsigned int> _slave_node_ids;
  // Holds the slave node set or side set
  std::string _slave_node_set_id;
  // Penalty if constrained is not satisfied
  Real _penalty;
  // vector of weights corresponding to master nodes for the linear combination
  std::vector<Real> _weights;
  // Specifies the formulation used to calculate residuals and jacobian
  std::string _formulation;
};

#endif /* LINEARNODALCONSTRAINT_H */
