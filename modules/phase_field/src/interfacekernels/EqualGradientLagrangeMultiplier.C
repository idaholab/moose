//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EqualGradientLagrangeMultiplier.h"

// MOOSE includes
#include "MooseVariable.h"

registerMooseObject("PhaseFieldApp", EqualGradientLagrangeMultiplier);

InputParameters
EqualGradientLagrangeMultiplier::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addClassDescription("Lagrange multiplier kernel for EqualGradientLagrangeInterface.");
  params.addRequiredParam<unsigned int>("component", "Gradient component to constrain");
  params.addCoupledVar("element_var",
                       "The gradient constrained variable on this side of the interface.");
  params.addParam<Real>("jacobian_fill",
                        0.0,
                        "Compensate on diagonal Jacobian fill term when "
                        "using a NullKernel on the Lagrange multiplier "
                        "variable");
  return params;
}

EqualGradientLagrangeMultiplier::EqualGradientLagrangeMultiplier(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _component(getParam<unsigned int>("component")),
    _grad_element_value(getVar("element_var", 0)->gradSln()),
    _element_jvar(getVar("element_var", 0)->number()),
    _neighbor_jvar(_neighbor_var.number()),
    _jacobian_fill(getParam<Real>("jacobian_fill"))
{
}

Real
EqualGradientLagrangeMultiplier::computeQpResidual(Moose::DGResidualType type)
{
  if (type == Moose::Element)
    return (_grad_element_value[_qp](_component) - _grad_neighbor_value[_qp](_component)) *
           _test[_i][_qp];

  return 0.0;
}

Real
EqualGradientLagrangeMultiplier::computeQpJacobian(Moose::DGJacobianType type)
{
  if (type == Moose::ElementNeighbor)
    return -_grad_phi_neighbor[_j][_qp](_component) * _test[_i][_qp];

  if (type == Moose::ElementElement)
    return -_jacobian_fill;

  return 0.0;
}

Real
EqualGradientLagrangeMultiplier::computeQpOffDiagJacobian(Moose::DGJacobianType type,
                                                          unsigned int jvar)
{
  if (type == Moose::ElementElement && jvar == _element_jvar)
    return _grad_phi[_j][_qp](_component) * _test[_i][_qp];

  return 0.0;
}
