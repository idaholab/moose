#include "EpsilonModelKernel1stV2Gauss.h"

registerMooseObject("PhaseFieldApp", EpsilonModelKernel1stV2Gauss);

InputParameters
EpsilonModelKernel1stV2Gauss::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription(
      "Implements the term: -L \\nabla \\cdot \\left( \\frac{1}{2} \\frac{\\partial "
      "\\epsilon(\\theta, v)}{\\partial \\nabla \\eta_i} \\sum_{i=1}^p (\\eta_i^2) \\right)");

  params.addRequiredCoupledVar("v",
                               "Array of coupled order parameter names for other order parameters");

  return params;
}

EpsilonModelKernel1stV2Gauss::EpsilonModelKernel1stV2Gauss(const InputParameters & parameters)
  : ADKernel(parameters),
    _L_AD(getADMaterialProperty<Real>("L_AD")),
    _eps(getADMaterialProperty<Real>("eps")),
    _deps_minus(getADMaterialProperty<RealGradient>("deps_minus")),
    _op_num(coupledComponents("v")),
    _vals(adCoupledValues("v")),
    _grad_vals(adCoupledGradients("v"))
{
}

ADReal
EpsilonModelKernel1stV2Gauss::computeQpResidual()
{
  // Compute the squared gradient.
  const ADReal SqrGrad = _grad_u[_qp] * _grad_u[_qp];

  // Sum of squared gradients of the coupled variables.
  ADReal SumSqrGradEtaj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumSqrGradEtaj += (*_grad_vals[i])[_qp] * (*_grad_vals[i])[_qp];

  const auto & grad_test = _grad_test[_i][_qp];

  return _L_AD[_qp] * 0.5 * (SqrGrad + SumSqrGradEtaj) * _deps_minus[_qp] * grad_test;
}
