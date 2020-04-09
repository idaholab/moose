//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GradientComponent.h"

registerMooseObject("PhaseFieldApp", GradientComponent);

InputParameters
GradientComponent::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Set the kernel variable to a specified component of the gradient of a coupled variable.");
  params.addRequiredCoupledVar("v", "Coupled variable to match gradient component of");
  params.addRequiredParam<unsigned int>("component",
                                        "Component of the gradient of the coupled variable v");
  return params;
}

GradientComponent::GradientComponent(const InputParameters & parameters)
  : Kernel(parameters),
    _v_var(coupled("v")),
    _grad_v(coupledGradient("v")),
    _component(getParam<unsigned int>("component"))
{
  if (_component >= LIBMESH_DIM)
    paramError("component", "Component too large for LIBMESH_DIM");
}

Real
GradientComponent::computeQpResidual()
{
  return (_u[_qp] - _grad_v[_qp](_component)) * _test[_i][_qp];
}

Real
GradientComponent::computeQpJacobian()
{
  return _phi[_j][_qp] * _test[_i][_qp];
}

Real
GradientComponent::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
    return -_grad_phi[_j][_qp](_component) * _test[_i][_qp];
  return 0.0;
}
