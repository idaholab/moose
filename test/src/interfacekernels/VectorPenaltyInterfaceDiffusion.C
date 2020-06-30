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

InputParameters
VectorPenaltyInterfaceDiffusion::validParams()
{
  InputParameters params = VectorInterfaceKernel::validParams();
  params.addClassDescription("A test VectorInterfaceKernel that imposes the condition: $\\vec{u} - "
                             "\\vec{v} = 0$ across an interface.");
  params.addRequiredParam<Real>(
      "penalty", "The penalty that penalizes jump between primary and neighbor variables.");
  return params;
}

VectorPenaltyInterfaceDiffusion::VectorPenaltyInterfaceDiffusion(const InputParameters & parameters)
  : VectorInterfaceKernel(parameters), _penalty(getParam<Real>("penalty"))
{
}

Real
VectorPenaltyInterfaceDiffusion::computeQpResidual(Moose::DGResidualType type)
{
  Real res = 0;

  switch (type)
  {
    case Moose::Element:
      res = _test[_i][_qp] * _penalty * (_u[_qp] - _neighbor_value[_qp]);
      break;

    case Moose::Neighbor:
      res = _test_neighbor[_i][_qp] * -_penalty * (_u[_qp] - _neighbor_value[_qp]);
      break;
  }

  return res;
}

Real
VectorPenaltyInterfaceDiffusion::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0;

  switch (type)
  {
    case Moose::ElementElement:
      jac = _test[_i][_qp] * _penalty * _phi[_j][_qp];
      break;

    case Moose::ElementNeighbor:
      jac = _test[_i][_qp] * _penalty * -_phi_neighbor[_j][_qp];
      break;

    case Moose::NeighborNeighbor:
      jac = _test_neighbor[_i][_qp] * -_penalty * -_phi_neighbor[_j][_qp];
      break;

    case Moose::NeighborElement:
      jac = _test_neighbor[_i][_qp] * -_penalty * _phi[_j][_qp];
      break;
  }

  return jac;
}
