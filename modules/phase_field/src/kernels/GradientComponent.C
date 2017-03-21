/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GradientComponent.h"

template <>
InputParameters
validParams<GradientComponent>()
{
  InputParameters params = validParams<Kernel>();
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
    mooseError("Component too large for LIBMESH_DIM");
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
