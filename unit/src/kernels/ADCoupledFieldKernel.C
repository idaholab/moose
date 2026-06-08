#include "ADCoupledFieldKernel.h"

registerMooseObject("MooseUnitApp", ADCoupledFieldKernel);

InputParameters
ADCoupledFieldKernel::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addRequiredCoupledVar("u", "The coupled field variable");
  params.addClassDescription(
      "Field residual source density: -1 / (1/(1+u) + 1/(1 + |grad(u)|^2)).");
  return params;
}

ADCoupledFieldKernel::ADCoupledFieldKernel(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _coupled_u(adCoupledValue("u")),
    _grad_coupled_u(adCoupledGradient("u"))
{
}

ADReal
ADCoupledFieldKernel::precomputeQpResidual()
{
  return -1.0 / (1.0 / (1.0 + _coupled_u[_qp]) +
                 1.0 / (1.0 + _grad_coupled_u[_qp] * _grad_coupled_u[_qp]));
}
