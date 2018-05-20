#include "CoupledCurlCurlField.h"

registerMooseObject("ElkApp", CoupledCurlCurlField);

template <>
InputParameters
validParams<CoupledCurlCurlField>()
{
  InputParameters params = validParams<VectorKernel>();
  params.addClassDescription(
      "Weak form term corresponding to $\\nabla \\times \\nabla \\times \\vec{E}$.");
  params.addParam<Real>("sign", 1.0, "Sign in weak form.");
  params.addRequiredCoupledVar("coupled", "Coupled variable.");
  return params;
}

CoupledCurlCurlField::CoupledCurlCurlField(const InputParameters & parameters)
  : VectorKernel(parameters), _sign(getParam<Real>("sign")), _coupled_curl(coupledCurl("coupled"))
{
}

Real
CoupledCurlCurlField::computeQpResidual()
{
  return _sign * _coupled_curl[_qp] * _curl_test[_i][_qp];
}

Real
CoupledCurlCurlField::computeQpJacobian()
{
  return 0.0;
}
