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
