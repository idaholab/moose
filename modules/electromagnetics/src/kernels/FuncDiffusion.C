#include "FuncDiffusion.h"
#include "Function.h"

registerMooseObject("ElectromagneticsApp", FuncDiffusion);

InputParameters
FuncDiffusion::validParams()
{
  InputParameters params = Diffusion::validParams();
  params.addClassDescription("The Laplacian operator with a function coefficient.");
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
