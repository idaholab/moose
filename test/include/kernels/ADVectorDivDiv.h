//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

#include "ADVectorKernel.h"

/**
 * AD grad-div operator (div(psi), k div(u)), with div(u) taken from the variable or from a
 * coupled vector variable, to test adDivSln() and adCoupledDiv()
 */
class ADVectorDivDiv : public ADVectorKernel
{
public:
  static InputParameters validParams();

  ADVectorDivDiv(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// div of the test function
  const VectorVariableTestDivergence & _div_test;
  /// div of the variable or of the coupled variable
  const ADVectorVariableDivergence & _div_v;
  /// scalar coefficient
  const Real _coeff;
};
