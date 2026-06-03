#include "ADCoupledScalarDiffusion.h"

registerMooseObject("MooseTestApp", ADCoupledScalarDiffusion);

InputParameters
ADCoupledScalarDiffusion::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Weak form of -div(v * grad(u)) where v is a coupled scalar variable.");
  params.addRequiredCoupledVar("v", "Coupled scalar variable used as diffusion coefficient.");
  return params;
}

ADCoupledScalarDiffusion::ADCoupledScalarDiffusion(const InputParameters & parameters)
  : ADKernel(parameters), _v(adCoupledScalarValue("v"))
{
}

ADReal
ADCoupledScalarDiffusion::computeQpResidual()
{
  return _v[0] * _grad_u[_qp] * _grad_test[_i][_qp];
}
