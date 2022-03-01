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
 * A CoupledTiedValueConstraint forces the value of a variable to be the same on both sides of an
 * interface.
 */
class CoupledTiedValueConstraint : public NodeFaceConstraint
{
public:
  static InputParameters validParams();

  CoupledTiedValueConstraint(const InputParameters & parameters);

protected:
  virtual Real computeQpSecondaryValue() override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;

  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                        unsigned int jvar) override;
  const Real _scaling;
  NumericVector<Number> & _residual_copy;
};
