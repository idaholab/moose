//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

/**
 * Base class for implementing constraints on boundaries for finite volume variables using scalar
 * Lagrange multipliers
 */
class FVBoundaryScalarLagrangeMultiplierConstraint : public FVFluxBC
{
public:
  static InputParameters validParams();

  FVBoundaryScalarLagrangeMultiplierConstraint(const InputParameters & parameters);

  const MooseVariableScalar & lambdaVariable() const { return _lambda_var; }

protected:
  /// The value that we want the average/point-value/etc. of the primal variable to be equal to
  const PostprocessorValue & _phi0;

private:
  void computeResidual(const FaceInfo & fi) override final;
  void computeJacobian(const FaceInfo & fi) override final;

  /// The Lagrange Multiplier variable
  const MooseVariableScalar & _lambda_var;

  /// The Lagrange Multiplier value
  const ADVariableValue & _lambda;
};
