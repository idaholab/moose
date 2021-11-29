//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodeFaceConstraint.h"

/**
 * A TiedValueConstraint forces the value of a variable to be the same on both sides of an
 * interface.
 */
class TiedValueConstraint : public NodeFaceConstraint
{
public:
  static InputParameters validParams();

  TiedValueConstraint(const InputParameters & parameters);

protected:
  virtual Real computeQpSecondaryValue() override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;

  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;
  const Real _scaling;
  NumericVector<Number> & _residual_copy;
};
