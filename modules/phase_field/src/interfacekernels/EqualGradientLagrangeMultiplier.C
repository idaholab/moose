/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "EqualGradientLagrangeMultiplier.h"

template<>
InputParameters validParams<EqualGradientLagrangeMultiplier>()
{
  InputParameters params = validParams<InterfaceKernel>();
  params.addClassDescription("Lagrange multiplier kernel for EqualGradientLagrangeInterface.");
  params.addRequiredParam<unsigned int>("component", "Gradient component to constrain");
  params.addCoupledVar("element_var", "The gradient constrained variable on this side of the interface.");
  return params;
}

EqualGradientLagrangeMultiplier::EqualGradientLagrangeMultiplier(const InputParameters & parameters) :
    InterfaceKernel(parameters),
    _component(getParam<unsigned int>("component")),
    _grad_element_value(getVar("element_var", 0)->gradSln()),
    _element_jvar(getVar("element_var", 0)->number()),
    _neighbor_jvar(_neighbor_var.number())
{
}

Real
EqualGradientLagrangeMultiplier::computeQpResidual(Moose::DGResidualType type)
{
  if (type == Moose::Element)
    return (_grad_element_value[_qp](_component) - _grad_neighbor_value[_qp](_component)) * _test[_i][_qp];

  return 0.0;
}

Real
EqualGradientLagrangeMultiplier::computeQpJacobian(Moose::DGJacobianType type)
{
  if (type == Moose::ElementNeighbor)
    return -_grad_phi_neighbor[_j][_qp](_component) * _test[_i][_qp];

  return 0.0;
}

Real
EqualGradientLagrangeMultiplier::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  if (type == Moose::ElementElement && jvar == _element_jvar)
    return _grad_phi[_j][_qp](_component) * _test[_i][_qp];

  return 0.0;
}
