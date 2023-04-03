//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDiffusionNoScalar.h"

registerMooseObject("MooseTestApp", ADDiffusionNoScalar);

InputParameters
ADDiffusionNoScalar::validParams()
{
  InputParameters params = ADKernelScalarBase::validParams();
  params.addClassDescription(
      "Same as `DiffusionNoScalar` in terms of physics/residual, but the Jacobian "
      "is computed using forward automatic differentiation.");
  return params;
}

ADDiffusionNoScalar::ADDiffusionNoScalar(const InputParameters & parameters)
  : ADKernelScalarBase(parameters)
{
}

ADReal
ADDiffusionNoScalar::computeQpResidual()
{
  return _grad_u[_qp] * _grad_test[_i][_qp];
}
