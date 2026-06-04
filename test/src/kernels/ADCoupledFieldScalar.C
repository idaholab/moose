#include "ADCoupledFieldScalar.h"

registerMooseObject("MooseTestApp", ADCoupledFieldScalar);

InputParameters
ADCoupledFieldScalar::validParams()
{
  InputParameters params = ElementADScalarKernel::validParams();
  params.addRequiredCoupledVar("u", "The coupled field variable");
  params.addClassDescription(
      "Scalar variable residual: integral of -1 / (1/(1+u) + 1/(1 + |grad(u)|^2)).");
  return params;
}

ADCoupledFieldScalar::ADCoupledFieldScalar(const InputParameters & parameters)
  : ElementADScalarKernel(parameters),
    _u(adCoupledValue("u")),
    _grad_u(adCoupledGradient("u"))
{
}

ADReal
ADCoupledFieldScalar::computeScalarQpResidual()
{
  return -1.0 / (1.0 / (1.0 + _u[_qp]) + 1.0 / (1.0 + _grad_u[_qp] * _grad_u[_qp]));
}
