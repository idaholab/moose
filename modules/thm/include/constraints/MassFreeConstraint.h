#pragma once

#include "NodalConstraint.h"

class MassFreeConstraint;

template <>
InputParameters validParams<MassFreeConstraint>();

/**
 * Free BC for the mass equation
 */
class MassFreeConstraint : public NodalConstraint
{
public:
  MassFreeConstraint(const InputParameters & parameters);

  virtual void computeResidual(NumericVector<Number> & residual);
  virtual void computeJacobian(SparseMatrix<Number> & jacobian);

protected:
  virtual Real computeQpResidual(Moose::ConstraintType type);
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type);

  std::vector<Real> _normals;
  const VariableValue & _rhouA;

  unsigned int _rhouA_var_number;
};
