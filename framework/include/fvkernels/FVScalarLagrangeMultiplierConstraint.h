//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

/**
 * Base class for implementing constraints on finite volume variable elemental values using scalar
 * Lagrange multipliers
 */
class FVScalarLagrangeMultiplierConstraint : public FVElementalKernel
{
public:
  static InputParameters validParams();

  FVScalarLagrangeMultiplierConstraint(const InputParameters & parameters);

  const MooseVariableScalar & lambdaVariable() const { return _lambda_var; }

protected:
  /// The value that we want the average of the primal variable to be equal to
  const PostprocessorValue & _phi0;

private:
  void computeResidual() override final;
  void computeJacobian() override final;
  using FVElementalKernel::computeOffDiagJacobian;
  void computeOffDiagJacobian() override final;
  void computeResidualAndJacobian() override final;
  ADReal computeQpResidual() override = 0;

  /// The Lagrange Multiplier variable
  const MooseVariableScalar & _lambda_var;

  /// The Lagrange Multiplier value
  const ADVariableValue & _lambda;
};
