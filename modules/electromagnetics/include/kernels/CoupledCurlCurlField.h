#ifndef COUPLEDCURLCURLFIELD_H
#define COUPLEDCURLCURLFIELD_H

#include "VectorKernel.h"

class CoupledCurlCurlField;

template <>
InputParameters validParams<CoupledCurlCurlField>();

class CoupledCurlCurlField : public VectorKernel
{
public:
  CoupledCurlCurlField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// curl of the test function
  const VectorVariableTestCurl & _curl_test;

  const VectorVariableCurl & _coupled_curl;

  Real _sign;
};

#endif // COUPLEDCURLCURLFIELD_H
