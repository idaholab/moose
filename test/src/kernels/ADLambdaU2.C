#include "ADLambdaU2.h"

registerMooseObject("MooseTestApp", ADLambdaU2);

InputParameters
ADLambdaU2::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Kernel residual: lambda * u^2.");
  params.addRequiredParam<Real>("lambda", "Scalar coefficient.");
  return params;
}

ADLambdaU2::ADLambdaU2(const InputParameters & parameters)
  : ADKernel(parameters), _lambda(getParam<Real>("lambda"))
{
}

ADReal
ADLambdaU2::computeQpResidual()
{
  return _lambda * _u[_qp] * _u[_qp] * _test[_i][_qp];
}
