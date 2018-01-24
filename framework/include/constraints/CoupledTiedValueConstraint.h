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

#ifndef COUPLEDTIEDVALUECONSTRAINT_H
#define COUPLEDTIEDVALUECONSTRAINT_H

// MOOSE includes
#include "NodeFaceConstraint.h"

// Forward Declarations
class CoupledTiedValueConstraint;

template <>
InputParameters validParams<CoupledTiedValueConstraint>();

/**
 * A CoupledTiedValueConstraint forces the value of a variable to be the same on both sides of an
 * interface.
 */
class CoupledTiedValueConstraint : public NodeFaceConstraint
{
public:
  CoupledTiedValueConstraint(const InputParameters & parameters);

protected:
  virtual Real computeQpSlaveValue() override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;

  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                        unsigned int jvar) override;
  const Real _scaling;
  NumericVector<Number> & _residual_copy;
};

#endif
