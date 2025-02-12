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
#include "MaterialProperty.h"

/**
 *  Supplies the curl(curl) operator of a vector variable and a
 *  corresponding forcing function using automatic differentiation
 */
class ADVectorFEWave : public ADVectorKernel
{
public:
  static InputParameters validParams();

  ADVectorFEWave(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// curl of the test function
  const VectorVariableTestCurl & _curl_test;

  /// Holds the solution curl at the current quadrature points
  const ADVectorVariableCurl & _curl_u;

  /// x component forcing function
  const Function & _x_ffn;
  /// y component forcing function
  const Function & _y_ffn;
  /// z component forcing function
  const Function & _z_ffn;
};
