/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef TIEDVALUECONSTRAINT_H
#define TIEDVALUECONSTRAINT_H

// MOOSE includes
#include "NodeFaceConstraint.h"

// Forward Declarations
class OneDContactConstraint;

template <>
InputParameters validParams<OneDContactConstraint>();

/**
 * A OneDContactConstraint forces the value of a variable to be the same on both sides of an
 * interface.
 */
class OneDContactConstraint : public NodeFaceConstraint
{
public:
  OneDContactConstraint(const InputParameters & parameters);
  virtual ~OneDContactConstraint() {}

  virtual void timestepSetup();
  virtual void jacobianSetup();

  virtual void updateContactSet();

  virtual Real computeQpSlaveValue();

  virtual Real computeQpResidual(Moose::ConstraintType type);

  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type);

  bool shouldApply();

protected:
  NumericVector<Number> & _residual_copy;

  bool _jacobian_update;
};

#endif
