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
