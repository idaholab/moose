//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
