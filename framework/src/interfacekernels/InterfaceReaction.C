//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceReaction.h"

registerMooseObject("MooseApp", InterfaceReaction);

InputParameters
InterfaceReaction::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addRequiredParam<Real>("kf", "Forward reaction rate coefficient.");
  params.addRequiredParam<Real>("kb", "Backward reaction rate coefficient.");
  params.addClassDescription("Implements a reaction to establish ReactionRate=k_f*u-k_b*v "
                             "at interface.");
  return params;
}

InterfaceReaction::InterfaceReaction(const InputParameters & parameters)
  : InterfaceKernel(parameters), _kf(getParam<Real>("kf")), _kb(getParam<Real>("kb"))
{
}

Real
InterfaceReaction::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;
  switch (type)
  {
    // Move all the terms to the LHS to get residual, for primary domain
    // Residual = kf*u - kb*v = kf*u - kb*v
    // Weak form for primary domain is: (test, kf*u - kb*v)
    case Moose::Element:
      r = _test[_i][_qp] * (_kf * _u[_qp] - _kb * _neighbor_value[_qp]);
      break;

    // Similarly, weak form for secondary domain is: -(test, kf*u - kb*v),
    // flip the sign because the direction is opposite.
    case Moose::Neighbor:
      r = -_test_neighbor[_i][_qp] * (_kf * _u[_qp] - _kb * _neighbor_value[_qp]);
      break;
  }
  return r;
}

Real
InterfaceReaction::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0;
  switch (type)
  {
    case Moose::ElementElement:
      jac = _test[_i][_qp] * _kf * _phi[_j][_qp];
      break;
    case Moose::NeighborNeighbor:
      jac = -_test_neighbor[_i][_qp] * -_kb * _phi_neighbor[_j][_qp];
      break;
    case Moose::NeighborElement:
      jac = -_test_neighbor[_i][_qp] * _kf * _phi[_j][_qp];
      break;
    case Moose::ElementNeighbor:
      jac = _test[_i][_qp] * -_kb * _phi_neighbor[_j][_qp];
      break;
  }
  return jac;
}
