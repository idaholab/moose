//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NullScalarKernel.h"

// MOOSE includes
#include "Assembly.h"

registerMooseObject("MooseApp", NullScalarKernel);

InputParameters
NullScalarKernel::validParams()
{
  InputParameters params = ScalarKernel::validParams();
  params.addClassDescription(
      "Scalar kernel that sets a zero residual, to avoid error from system missing this variable.");
  params.addParam<Real>("jacobian_fill",
                        0.0,
                        "On diagonal Jacobian fill term, potentially needed for preconditioner");
  return params;
}

NullScalarKernel::NullScalarKernel(const InputParameters & parameters)
  : ScalarKernel(parameters), _jacobian_fill(getParam<Real>("jacobian_fill"))
{
}

void
NullScalarKernel::reinit()
{
}

Real
NullScalarKernel::computeQpResidual()
{
  return 0;
}

Real
NullScalarKernel::computeQpJacobian()
{
  return _jacobian_fill;
}
