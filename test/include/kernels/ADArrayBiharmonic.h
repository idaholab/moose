//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADArrayKernel.h"

/**
 * Test kernel for the automatic differentiation second derivatives of array variables. Adds the
 * biharmonic residual (laplacian(v), laplacian(u^*)) component by component, where v is a coupled
 * array variable, or the kernel variable if v is not given.
 */
class ADArrayBiharmonic : public ADArrayKernel
{
public:
  static InputParameters validParams();

  ADArrayBiharmonic(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(ADRealEigenVector & residual) override;

  /// second derivatives of the coupled (or own) array variable
  const ADArrayVariableSecond & _second_v;
  /// second derivatives of the test functions
  const ArrayVariableTestSecond & _second_test;
};
