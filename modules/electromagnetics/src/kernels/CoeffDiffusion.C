#include "CoeffDiffusion.h"

template<>
InputParameters validParams<CoeffDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<Real>("coefficient", "Coefficient multiplier for diffusion term.");
  return params;
}

CoeffDiffusion::CoeffDiffusion(const InputParameters & parameters)
  : Diffusion(parameters),

  _coefficient(getParam<Real>("coefficient"))

{}

Real
CoeffDiffusion::computeQpResidual()
{
  return _coefficient * Diffusion::computeQpResidual();
}

Real
CoeffDiffusion::computeQpJacobian()
{
  return _coefficient * Diffusion::computeQpJacobian();
}
