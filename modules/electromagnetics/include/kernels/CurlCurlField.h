#ifndef CURLCURLFIELD_H
#define CURLCURLFIELD_H

#include "VectorKernel.h"

class CurlCurlField;

template <>
InputParameters validParams<CurlCurlField>();

class CurlCurlField : public VectorKernel
{
public:
  CurlCurlField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// curl of the test function
  const VectorVariableTestCurl & _curl_test;

  /// curl of the shape function
  const VectorVariablePhiCurl & _curl_phi;

  /// Holds the solution curl at the current quadrature points
  const VectorVariableCurl & _curl_u;

  Real _coeff;
};

#endif // CURLCURLFIELD_H
