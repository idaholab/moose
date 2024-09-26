//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EpsilonModelEpsilonGradientKernel.h"

registerMooseObject("PhaseFieldApp", EpsilonModelEpsilonGradientKernel);

InputParameters
EpsilonModelEpsilonGradientKernel::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription(
      "Implements the epsilon-gradient term: -L \\nabla \\cdot \\left( \\frac{1}{2} \\frac{\\partial "
      "\\epsilon(\\theta, v)}{\\partial \\nabla \\eta_i} \\sum_{i=1}^p (\\eta_i^2) \\right)");
  params.addRequiredCoupledVar("v",
                               "Array of coupled order parameter names for other order parameters");
  MooseEnum sets("FIRST SECOND");
  params.addRequiredParam<MooseEnum>("grains_set", sets, "First or Second set of grains?");
  return params;
}

EpsilonModelEpsilonGradientKernel::EpsilonModelEpsilonGradientKernel(const InputParameters & parameters)
  : ADKernel(parameters),
    _grains_set(getParam<MooseEnum>("grains_set").getEnum<SetsType>()),
    _depsilon_plus(getADMaterialProperty<RealGradient>("depsilon_plus")),
    _depsilon_minus(getADMaterialProperty<RealGradient>("depsilon_minus")),
    _L(getADMaterialProperty<Real>("L")),
    _op_num(coupledComponents("v")),
    _grad_vals(adCoupledGradients("v"))
{
}

ADReal
EpsilonModelEpsilonGradientKernel::computeQpResidual()
{
  // Compute the squared gradient of the primary variable
  const ADReal SqrGrad = _grad_u[_qp] * _grad_u[_qp];

  // Sum of squared gradients of the coupled variables
  ADReal SumSqrGradEtaj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumSqrGradEtaj += (*_grad_vals[i])[_qp] * (*_grad_vals[i])[_qp];

  // Reference to the gradient test function at this quadrature point
  const auto & grad_test = _grad_test[_i][_qp];

  // Evaluate the expression based on _grains_set
  if (_grains_set == SetsType::FIRST)  // If grains_set is "FIRST"
    return 0.5 * _L[_qp] * (SqrGrad + SumSqrGradEtaj) * _depsilon_plus[_qp] * grad_test;
  else if (_grains_set == SetsType::SECOND)  // If grains_set is "SECOND"
    return 0.5 * _L[_qp] * (SqrGrad + SumSqrGradEtaj) * _depsilon_minus[_qp] * grad_test;
  else
    mooseError("Invalid grains_set value"); // Handle unexpected values of _grains_set
}
