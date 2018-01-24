//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeAdvection.h"

template <>
InputParameters
validParams<ConservativeAdvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Conservative form of $\\nabla \\cdot \\vec{v} u$ which in its weak "
                             "form is given by: $(-\\nabla \\psi_i, \\vec{v} u)$.");
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity vector");
  return params;
}

ConservativeAdvection::ConservativeAdvection(const InputParameters & parameters)
  : Kernel(parameters), _velocity(getParam<RealVectorValue>("velocity"))
{
}

Real
ConservativeAdvection::computeQpResidual()
{
  return -_u[_qp] * (_velocity * _grad_test[_i][_qp]);
}

Real
ConservativeAdvection::computeQpJacobian()
{
  return -_phi[_j][_qp] * (_velocity * _grad_test[_i][_qp]);
}
