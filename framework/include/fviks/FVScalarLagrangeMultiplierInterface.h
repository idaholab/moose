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

protected:
  /**
   * compute the AD residuals and feed the results into the supplied vector and matrix tags
   */
  void computeResidual(const FaceInfo & fi,
                       const std::set<TagID> & vector_tags,
                       const std::set<TagID> & matrix_tags) override final;

  ADReal computeQpResidual() override = 0;

  /// The Lagrange Multiplier variable
  const MooseVariableScalar & _lambda_var;

  /// The Lagrange Multiplier value
  const ADVariableValue & _lambda;
};
