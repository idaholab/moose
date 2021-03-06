#include "CoupledCoeffField.h"
#include "Function.h"

registerMooseObject("ElkApp", CoupledCoeffField);

InputParameters
CoupledCoeffField::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Kernel representing the contribution of the PDE term $cfv$, where $c$ and $f$ are constant "
      "and function coefficients, respectively, and $v$ is a coupled scalar variable.");
  params.addParam<Real>("coeff", 1.0, "Coefficient multiplier for field.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for field.");
  params.addRequiredCoupledVar("coupled_field", "Coupled field variable.");
  params.addParam<Real>("sign", 1.0, "Sign of term in weak form.");
  return params;
}

CoupledCoeffField::CoupledCoeffField(const InputParameters & parameters)
  : Kernel(parameters),

    _coefficient(getParam<Real>("coeff")),

    _func(getFunction("func")),

    _coupled_val(coupledValue("coupled_field")),

    _sign(getParam<Real>("sign"))

{
}

Real
CoupledCoeffField::computeQpResidual()
{
  return _sign * _coefficient * _func.value(_t, _q_point[_qp]) * _test[_i][_qp] * _coupled_val[_qp];
}

Real
CoupledCoeffField::computeQpJacobian()
{
  return 0;
}

// TODO: missing off-diagonal Jacobian contribution!
