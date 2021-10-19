//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorKernel.h"

class CoupledCurlCurlField : public VectorKernel
{
public:
  static InputParameters validParams();

  CoupledCurlCurlField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// curl of the test function
  const VectorVariableTestCurl & _curl_test;

  const VectorVariableCurl & _coupled_curl;

  Real _sign;
};
