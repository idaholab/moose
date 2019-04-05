#include "FuncDiffusion.h"
#include "Function.h"

registerMooseObject("ElkApp", FuncDiffusion);

template <>
InputParameters
validParams<FuncDiffusion>()
{
  InputParameters params = validParams<Diffusion>();
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for diffusion term.");
  return params;
}

FuncDiffusion::FuncDiffusion(const InputParameters & parameters)
  : Diffusion(parameters),

    _func(getFunction("func"))

{
}

Real
FuncDiffusion::computeQpResidual()
{
  return _func.value(_t, _q_point[_qp]) * Diffusion::computeQpResidual();
}

Real
FuncDiffusion::computeQpJacobian()
{
  return _func.value(_t, _q_point[_qp]) * Diffusion::computeQpJacobian();
}
