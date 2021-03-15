#include "CoupledCurlCurlField.h"

registerMooseObject("ElkApp", CoupledCurlCurlField);

InputParameters
CoupledCurlCurlField::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addClassDescription(
      "Weak form term corresponding to the curl of the curl of a coupled vector variable.");
  params.addParam<Real>("sign", 1.0, "Sign in weak form.");
  params.addRequiredCoupledVar("coupled", "Coupled variable.");
  return params;
}

CoupledCurlCurlField::CoupledCurlCurlField(const InputParameters & parameters)
  : VectorKernel(parameters),
    _curl_test(_var.curlPhi()),
    _coupled_curl(coupledCurl("coupled")),
    _sign(getParam<Real>("sign"))
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
