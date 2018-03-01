#include "CoupledCoeffDiffusion.h"
#include "Function.h"

template <>
InputParameters
validParams<CoupledCoeffDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<Real>("coefficient", 1.0, "Coefficient multiplier for diffusion term.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for diffusion term.");
  params.addParam<Real>("sign", 1.0, "Sign of Kernel, if it needs to be changed.");
  params.addRequiredCoupledVar("coupled_field", "Coupled field variable.");
  return params;
}

CoupledCoeffDiffusion::CoupledCoeffDiffusion(const InputParameters & parameters)
  : Kernel(parameters),

    _coefficient(getParam<Real>("coefficient")),
    _func(getFunction("func")),
    _sign(getParam<Real>("sign")),
    _coupled_grad(coupledGradient("coupled_field"))

{
}

Real
CoupledCoeffDiffusion::computeQpResidual()
{
  return _sign * _coefficient * _func.value(_t, _q_point[_qp]) * _grad_test[_i][_qp] *
         _coupled_grad[_qp];
}

Real
CoupledCoeffDiffusion::computeQpJacobian()
{
  return 0;
}
