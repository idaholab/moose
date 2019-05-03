//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPenaltyInterfaceDiffusion.h"

registerMooseObject("MooseTestApp", VectorPenaltyInterfaceDiffusion);

template <>
InputParameters
validParams<VectorPenaltyInterfaceDiffusion>()
{
  InputParameters params = validParams<VectorInterfaceKernel>();
  params.addClassDescription("A test VectorInterfaceKernel that imposes the condition: $\\vec{u} - \\vec{v} = 0$ across an interface.");
  params.addRequiredParam<Real>("penalty", "The penalty that penalizes jump between master and neighbor variables.");
  return params;
}

VectorPenaltyInterfaceDiffusion::VectorPenaltyInterfaceDiffusion(const InputParameters & parameters)
  : VectorInterfaceKernel(parameters), _penalty(getParam<Real>("penalty"))
{
}

Real
VectorPenaltyInterfaceDiffusion::computeQpResidual(Moose::DGResidualType type)
{
  switch (type)
  {
    case Moose::Element:
      return _test[_i][_qp] * _penalty * (_u[_qp] - _neighbor_value[_qp]);

    case Moose::Neighbor:
      return _test_neighbor[_i][_qp] * -_penalty * (_u[_qp] - _neighbor_value[_qp]);
  }
}

Real
VectorPenaltyInterfaceDiffusion::computeQpJacobian(Moose::DGJacobianType type)
{
  switch (type)
  {
    case Moose::ElementElement:
      return _test[_i][_qp] * _penalty * _phi[_j][_qp];

    case Moose::ElementNeighbor:
      return _test[_i][_qp] * _penalty * -_phi_neighbor[_j][_qp];

    case Moose::NeighborNeighbor:
      return _test_neighbor[_i][_qp] * -_penalty * -_phi_neighbor[_j][_qp];

    case Moose::NeighborElement:
      return _test_neighbor[_i][_qp] * -_penalty * _phi[_j][_qp];
  }
}
