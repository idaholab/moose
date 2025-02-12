//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVectorKernel.h"

/**
 *  Weak form contribution corresponding to the curl(curl(E)) where E is the
 *  electric field vector using Automatic Differentiation for the Jacobian
 */
class ADCurlCurlField : public ADVectorKernel
{
public:
  static InputParameters validParams();

  ADCurlCurlField(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// curl of the test function
  const VectorVariableTestCurl & _curl_test;

  /// Holds the solution curl at the current quadrature points
  const ADVectorVariableCurl & _curl_u;

  /// Scalar coefficient
  Real _coeff;
};
