#include "NonlinearDiffusion.h"

registerMooseObject("PhaseFieldApp", NonlinearDiffusion);

InputParameters
NonlinearDiffusion::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Compute the nonlinear diffusion term");
  params.addRequiredParam<std::string>("D", "diffusivity for the diffusion equation");
  return params;
}

NonlinearDiffusion::NonlinearDiffusion(const InputParameters & parameters)
  : ADKernel(parameters),
    _diffusivity_name(getParam<std::string>("D")),
    _diffusivity(getADMaterialProperty<Real>(_diffusivity_name))
{
}

ADReal
NonlinearDiffusion::computeQpResidual()
{
  return _diffusivity[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
}