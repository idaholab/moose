#include "CurlCurlField.h"

registerMooseObject("ElkApp", CurlCurlField);

template <>
InputParameters
validParams<CurlCurlField>()
{
  InputParameters params = validParams<VectorKernel>();
  params.addClassDescription(
      "Weak form term corresponding to $\\nabla \\times \\nabla \\times \\vec{E}$.");
  return params;
}

CurlCurlField::CurlCurlField(const InputParameters & parameters) : VectorKernel(parameters) {}

Real
CurlCurlField::computeQpResidual()
{
  return _curl_u[_qp] * _curl_test[_i][_qp];
}

Real
CurlCurlField::computeQpJacobian()
{
  return _curl_phi[_j][_qp] * _curl_test[_i][_qp];
}
