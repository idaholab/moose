//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVInterfaceKernel.h"

class FVScalarLagrangeMultiplierInterface : public FVInterfaceKernel
{
public:
  static InputParameters validParams();
  FVScalarLagrangeMultiplierInterface(const InputParameters & params);

  void computeResidual(const FaceInfo & fi) override final;
  void computeJacobian(const FaceInfo & fi) override final;

  /**
   * @return the Lagrange Multiplier scalar variable that enforces the interface constraint
   */
  const MooseVariableScalar & lambdaVariable() const { return _lambda_var; }

protected:
  ADReal computeQpResidual() override = 0;

  /// The Lagrange Multiplier variable
  const MooseVariableScalar & _lambda_var;

  /// The Lagrange Multiplier value
  const ADVariableValue & _lambda;
};
