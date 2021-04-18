#include "VectorCoeffField.h"
#include "Function.h"

registerMooseObject("ElectromagneticsApp", VectorCoeffField);

InputParameters
VectorCoeffField::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addClassDescription(
      "Kernel representing the contribution of the PDE term $cfu$, where $c$ and $f$ are constant "
      "and function coefficients, respectively, and $u$ is a vector variable.");
  params.addParam<Real>("coeff", 1.0, "Coefficient multiplier for field.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for field.");
  return params;
}

VectorCoeffField::VectorCoeffField(const InputParameters & parameters)
  : VectorKernel(parameters),

    _coefficient(getParam<Real>("coeff")),

    _func(getFunction("func"))
{
}

Real
VectorCoeffField::computeQpResidual()
{
  return _coefficient * _func.value(_t, _q_point[_qp]) * _u[_qp] * _test[_i][_qp];
}

Real
VectorCoeffField::computeQpJacobian()
{
  return _coefficient * _func.value(_t, _q_point[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
}
