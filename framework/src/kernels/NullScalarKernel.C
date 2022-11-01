//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NullScalarKernel.h"

registerMooseObject("MooseTestApp", NullScalarKernel);

InputParameters
NullScalarKernel::validParams()
{
  InputParameters params = ScalarKernel::validParams();
  params.addClassDescription("Scalar Kernel that sets a zero residual, to avoid error from system missing this variable.");
  params.addParam<Real>(
      "jacobian_fill",
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

void
NullScalarKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  for (_i = 0; _i < re.size(); _i++)
    re(_i) += 0.0;
}

void
NullScalarKernel::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  for (_i = 0; _i < ke.m(); _i++)
      ke(_i, _i) += _jacobian_fill;
}
