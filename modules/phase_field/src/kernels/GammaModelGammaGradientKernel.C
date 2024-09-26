//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GammaModelGammaGradientKernel.h"

registerMooseObject("PhaseFieldApp", GammaModelGammaGradientKernel);

InputParameters
GammaModelGammaGradientKernel::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Implements the term: -L m \\nabla \\cdot \\left( \\sum_{j \\neq i} "
                             "\\left( \\frac{\\partial \\gamma_{ij}}{\\partial \\nabla \\eta_i} "
                             "\\right) \\eta_i^2 \\eta_j^2 \\right)");
  params.addRequiredCoupledVar("v",
                               "Array of coupled order parameter names for other order parameters");
  MooseEnum sets("FIRST SECOND");
  params.addRequiredParam<MooseEnum>("grains_set", sets, "First or Second set of grains?");
  return params;
}

GammaModelGammaGradientKernel::GammaModelGammaGradientKernel(const InputParameters & parameters)
  : ADKernel(parameters),
    _grains_set(getParam<MooseEnum>("grains_set").getEnum<SetsType>()),
    _dgamma_plus(getADMaterialProperty<RealGradient>("dgamma_plus")),
    _dgamma_minus(getADMaterialProperty<RealGradient>("dgamma_minus")),
    _L(getADMaterialProperty<Real>("L")),
    _mu(getADMaterialProperty<Real>("mu")),
    _op_num(coupledComponents("v")),
    _vals(adCoupledValues("v"))
{
}

ADReal
GammaModelGammaGradientKernel::computeQpResidual()
{
  // Sum of squares of all other order parameters
  ADReal sum_eta_j = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    sum_eta_j += (*_vals[i])[_qp] * (*_vals[i])[_qp];

  // Reference to the gradient test function at this quadrature point
  const auto & grad_test = _grad_test[_i][_qp];

  // Evaluate the expression based on _grains_set
  if (_grains_set == SetsType::FIRST)  // If grains_set is "FIRST"
    return _L[_qp] * _mu[_qp] * sum_eta_j * _u[_qp] * _u[_qp] * _dgamma_plus[_qp] * grad_test;
  else if (_grains_set == SetsType::SECOND)  // If grains_set is "SECOND"
    return _L[_qp] * _mu[_qp] * sum_eta_j * _u[_qp] * _u[_qp] * _dgamma_minus[_qp] * grad_test;
  else
    mooseError("Invalid grains_set value"); // Handle unexpected values of _grains_set
}
