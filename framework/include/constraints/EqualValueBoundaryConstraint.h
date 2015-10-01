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

template<>
InputParameters validParams<EqualValueBoundaryConstraint>();

class EqualValueBoundaryConstraint : public NodalConstraint
{
public:
  EqualValueBoundaryConstraint(const InputParameters & parameters);
  virtual ~EqualValueBoundaryConstraint();

protected:

  /**
   * Computes the residual for the current slave node
   */
  virtual Real computeQpResidual(Moose::ConstraintType type, NumericVector<Number> & residual);

  /**
   * Computes the jacobian for the constraint
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type, SparseMatrix<Number> & jacobian);

  /**
   * Obtains the master node id and stores std::vector<dof_id_type>
   */

  unsigned int _master_node_id;
  short _slave_boundary_id;
  Real _penalty;
  // obtain the formulation used to calculated residuals and jacobian
  std::string _formulation;
};

#endif /* EQUALVALUEBOUNDARYCONSTRAINT_H */
