//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

class MooseVariableScalar;

/**
 * Enforces the average value of a finite element variable on a boundary using a scalar Lagrange
 * multiplier.
 *
 * This object owns both sides of the saddle-point constraint:
 * R_u += int_Gamma lambda test_i dGamma and R_lambda += int_Gamma (u - phi0) dGamma.
 */
class BoundaryIntegralValueConstraint : public IntegratedBC
{
public:
  static InputParameters validParams();

  BoundaryIntegralValueConstraint(const InputParameters & parameters);

  const MooseVariableScalar & lambdaVariable() const { return _lambda_var; }

  virtual std::set<std::string> additionalROVariables() override;

protected:
  virtual Real computeQpResidual() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;
  virtual void computeResidualAndJacobian() override;

  /// Compute the Lagrange multiplier residual contribution
  void computeScalarResidual();

  /// Compute the zero diagonal block for the Lagrange multiplier equation
  void computeScalarJacobian();

  /// Compute the Jacobian contribution from the Lagrange multiplier to the field equation
  void computeFieldScalarJacobian();

  /// Compute the Jacobian contribution from the field variable to the Lagrange multiplier equation
  void computeScalarFieldJacobian(unsigned int jvar);

  /// The value that the boundary average of the field variable is constrained to
  const PostprocessorValue & _phi0;

  /// Lagrange multiplier scalar variable
  const MooseVariableScalar & _lambda_var;

  /// Lagrange multiplier scalar variable value
  const VariableValue & _lambda;
};
