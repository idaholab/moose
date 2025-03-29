//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EpsilonModelMGradientKernel.h"

registerMooseObject("PhaseFieldApp", EpsilonModelMGradientKernel);

InputParameters
EpsilonModelMGradientKernel::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription(
      "Implements the m-gradient term: -L \\nabla \\cdot \\left( \\frac{\\partial "
      "m(\\theta, v)}{\\partial \\nabla \\eta_i} f_0 \\right)");
  MooseEnum sets("FIRST SECOND");
  params.addRequiredParam<MooseEnum>("grains_set", sets, "First or Second set of grains?");
  return params;
}

EpsilonModelMGradientKernel::EpsilonModelMGradientKernel(const InputParameters & parameters)
  : ADKernel(parameters),
    _grains_set(getParam<MooseEnum>("grains_set").getEnum<SetsType>()),
    _dm_plus(getADMaterialProperty<RealGradient>("dm_plus")),
    _dm_minus(getADMaterialProperty<RealGradient>("dm_minus")),
    _L(getADMaterialProperty<Real>("L")),
    _F(getADMaterialProperty<Real>("F"))
{
}

ADReal
EpsilonModelMGradientKernel::computeQpResidual()
{
  // Reference to the gradient test function at this quadrature point
  const auto & grad_test = _grad_test[_i][_qp];

  // Evaluate the expression based on _grains_set
  if (_grains_set == SetsType::FIRST) // If grains_set is "FIRST"
    return _L[_qp] * _F[_qp] * _dm_plus[_qp] * grad_test;
  else if (_grains_set == SetsType::SECOND) // If grains_set is "SECOND"
    return _L[_qp] * _F[_qp] * _dm_minus[_qp] * grad_test;
  else
    mooseError("Invalid grains_set value"); // Handle unexpected values of _grains_set
}
