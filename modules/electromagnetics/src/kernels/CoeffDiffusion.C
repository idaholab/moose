#include "CoeffDiffusion.h"
#include "Function.h"

registerMooseObject("ElkApp", CoeffDiffusion);

template <>
InputParameters
validParams<CoeffDiffusion>()
{
  InputParameters params = validParams<Diffusion>();
  params.addParam<Real>("coefficient", 1.0, "Coefficient multiplier for diffusion term.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for diffusion term.");
  return params;
}

CoeffDiffusion::CoeffDiffusion(const InputParameters & parameters)
  : Diffusion(parameters),

    _coefficient(getParam<Real>("coefficient")),
    _func(getFunction("func"))

{
}

Real
CoeffDiffusion::computeQpResidual()
{
  return _coefficient * _func.value(_t, _q_point[_qp]) * Diffusion::computeQpResidual();
}

Real
CoeffDiffusion::computeQpJacobian()
{
  return _coefficient * _func.value(_t, _q_point[_qp]) * Diffusion::computeQpJacobian();
}
