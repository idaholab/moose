#include "GammaModelKernelGauss.h"

registerMooseObject("PhaseFieldApp", GammaModelKernelGauss);

InputParameters
GammaModelKernelGauss::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Implements the term: -L m \\nabla \\cdot \\left( \\sum_{j \\neq i} "
                             "\\left( \\frac{\\partial \\gamma_{ij}}{\\partial \\nabla \\eta_i} "
                             "\\right) \\eta_i^2 \\eta_j^2 \\right)");
  params.addRequiredCoupledVar("v",
                               "Array of coupled order parameter names for other order parameters");
  return params;
}

GammaModelKernelGauss::GammaModelKernelGauss(const InputParameters & parameters)
  : ADKernel(parameters),
    _L_AD(getADMaterialProperty<Real>("L_AD")),
    _m(getADMaterialProperty<Real>("m")),
    _dgamma_plus(getADMaterialProperty<RealGradient>("dgamma_plus")),
    _op_num(coupledComponents("v")),
    _vals(adCoupledValues("v"))
{
}

ADReal
GammaModelKernelGauss::computeQpResidual()
{
  // Sum of squares of all other order parameters.
  ADReal sum_eta_j = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    sum_eta_j += (*_vals[i])[_qp] * (*_vals[i])[_qp];

  const auto & grad_test = _grad_test[_i][_qp];

  return _L_AD[_qp] * _m[_qp] * sum_eta_j * _u[_qp] * _u[_qp] * _dgamma_plus[_qp] * grad_test;
}
