#include "CurlCurlField.h"

registerMooseObject("ElkApp", CurlCurlField);

template <>
InputParameters
validParams<CurlCurlField>()
{
  InputParameters params = validParams<VectorKernel>();
  params.addClassDescription(
      "Weak form term corresponding to $\\nabla \\times \\nabla \\times \\vec{E}$.");
  params.addParam<Real>("sign", 1.0, "Sign in weak form.");
  return params;
}

CurlCurlField::CurlCurlField(const InputParameters & parameters)
  : VectorKernel(parameters), _sign(getParam<Real>("sign"))
{
}

Real
CurlCurlField::computeQpResidual()
{
  return _sign * _curl_u[_qp] * _curl_test[_i][_qp];
}

Real
CurlCurlField::computeQpJacobian()
{
  return _sign * _curl_phi[_j][_qp] * _curl_test[_i][_qp];
}
