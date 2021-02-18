#include "CoupledVectorCoeffField.h"
#include "Function.h"

registerMooseObject("ElkApp", CoupledVectorCoeffField);

template <>
InputParameters
validParams<CoupledVectorCoeffField>()
{
  InputParameters params = validParams<VectorKernel>();
  params.addClassDescription(
      "Kernel representing the contribution of the PDE term $cfv$, where $c$ and $f$ are constant "
      "and function coefficients, respectively, and $v$ is a coupled vector variable.");
  params.addParam<Real>("coeff", 1.0, "Coefficient multiplier for field.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for field.");
  params.addRequiredCoupledVar("coupled", "Coupled variable.");
  return params;
}

CoupledVectorCoeffField::CoupledVectorCoeffField(const InputParameters & parameters)
  : VectorKernel(parameters),

    _coefficient(getParam<Real>("coeff")),

    _func(getFunction("func")),

    _coupled_val(coupledVectorValue("coupled"))
{
}

Real
CoupledVectorCoeffField::computeQpResidual()
{
  return _coefficient * _func.value(_t, _q_point[_qp]) * _coupled_val[_qp] * _test[_i][_qp];
}

Real
CoupledVectorCoeffField::computeQpJacobian()
{
  return 0.0;
}
