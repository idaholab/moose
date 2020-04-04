//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EqualGradientLagrangeInterface.h"

// MOOSE includes
#include "MooseVariable.h"

registerMooseObject("PhaseFieldApp", EqualGradientLagrangeInterface);

InputParameters
EqualGradientLagrangeInterface::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addClassDescription("Enforce componentwise gradient continuity between two different "
                             "variables across a subdomain boundary using a Lagrange multiplier");
  params.addRequiredParam<unsigned int>("component", "Gradient component to constrain");
  params.addCoupledVar("lambda",
                       "The gradient constrained variable on this side of the interface.");
  return params;
}

EqualGradientLagrangeInterface::EqualGradientLagrangeInterface(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _component(getParam<unsigned int>("component")),
    _lambda(getVar("lambda", 0)->sln()),
    _lambda_jvar(getVar("lambda", 0)->number())
{
}

Real
EqualGradientLagrangeInterface::computeQpResidual(Moose::DGResidualType type)
{
  switch (type)
  {
    case Moose::Element:
      return _lambda[_qp] * _grad_test[_i][_qp](_component);

    case Moose::Neighbor:
      return -_lambda[_qp] * _grad_test_neighbor[_i][_qp](_component);
  }

  mooseError("Internal error.");
}

Real EqualGradientLagrangeInterface::computeQpJacobian(Moose::DGJacobianType /*type*/)
{
  return 0.0;
}

Real
EqualGradientLagrangeInterface::computeQpOffDiagJacobian(Moose::DGJacobianType type,
                                                         unsigned int jvar)
{
  if (jvar != _lambda_jvar)
    return 0.0;

  // lambda is only solved on the element side
  switch (type)
  {
    case Moose::ElementElement:
      return _phi[_j][_qp] * _grad_test[_i][_qp](_component);

    case Moose::NeighborElement:
      return -_phi[_j][_qp] * _grad_test_neighbor[_i][_qp](_component);

    default:
      return 0.0;
  }
}
