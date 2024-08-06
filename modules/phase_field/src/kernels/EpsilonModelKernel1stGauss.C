#include "EpsilonModelKernel1stGauss.h"

registerMooseObject("PhaseFieldApp", EpsilonModelKernel1stGauss);

InputParameters
EpsilonModelKernel1stGauss::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription(
      "Implements the term: -L \\nabla \\cdot \\left( \\frac{1}{2} \\frac{\\partial "
      "\\epsilon(\\theta, v)}{\\partial \\nabla \\eta_i} \\sum_{i=1}^p (\\eta_i^2) \\right)");

  params.addRequiredCoupledVar("v",
                               "Array of coupled order parameter names for other order parameters");

  return params;
}

EpsilonModelKernel1stGauss::EpsilonModelKernel1stGauss(const InputParameters & parameters)
  : ADKernel(parameters),
    _L_AD(getADMaterialProperty<Real>("L_AD")),
    _eps(getADMaterialProperty<Real>("eps")),
    _deps_plus(getADMaterialProperty<RealGradient>("deps_plus")),
    _op_num(coupledComponents("v")),
    _vals(adCoupledValues("v")),
    _grad_vals(adCoupledGradients("v"))
{
}

ADReal
EpsilonModelKernel1stGauss::computeQpResidual()
{
  // Compute the squared gradient.
  const ADReal SqrGrad = _grad_u[_qp] * _grad_u[_qp];

  // Sum of squared gradients of the coupled variables.
  ADReal SumSqrGradEtaj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumSqrGradEtaj += (*_grad_vals[i])[_qp] * (*_grad_vals[i])[_qp];

  const auto & grad_test = _grad_test[_i][_qp];

  return _L_AD[_qp] * 0.5 * (SqrGrad + SumSqrGradEtaj) * _deps_plus[_qp] * grad_test;
}
