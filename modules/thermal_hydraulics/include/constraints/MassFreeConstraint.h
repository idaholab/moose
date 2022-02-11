//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalConstraint.h"

/**
 * Free BC for the mass equation
 */
class MassFreeConstraint : public NodalConstraint
{
public:
  MassFreeConstraint(const InputParameters & parameters);

  virtual void computeResidual(NumericVector<Number> & residual) override;
  using NodalConstraint::computeResidual;
  virtual void computeJacobian(SparseMatrix<Number> & jacobian) override;
  using NodalConstraint::computeJacobian;

protected:
  virtual Real computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  std::vector<Real> _normals;
  const VariableValue & _rhouA;

  unsigned int _rhouA_var_number;

public:
  static InputParameters validParams();
};
