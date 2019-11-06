//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffMKernel.h"

registerMooseObject("MooseTestApp", DiffMKernel);

InputParameters
DiffMKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "mat_prop", "the name of the material property we are going to use");
  params.addParam<Real>("offset", 4.0, "Offset on residual evaluation");
  return params;
}

DiffMKernel::DiffMKernel(const InputParameters & parameters)
  : Kernel(parameters),
    _diff(getMaterialProperty<Real>("mat_prop")),
    _offset(getParam<Real>("offset"))
{
}

Real
DiffMKernel::computeQpResidual()
{
  return _diff[_qp] * _grad_test[_i][_qp] * _grad_u[_qp] - _offset;
}

Real
DiffMKernel::computeQpJacobian()
{
  return _diff[_qp] * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
