//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceDiffusionFluxMatch.h"

registerMooseObject("PhaseFieldApp", InterfaceDiffusionFluxMatch);

InputParameters
InterfaceDiffusionFluxMatch::validParams()
{
  InputParameters params = InterfaceDiffusionBase::validParams();
  params.addClassDescription(
      "Enforce flux continuity between two different variables across a subdomain boundary");
  return params;
}

InterfaceDiffusionFluxMatch::InterfaceDiffusionFluxMatch(const InputParameters & parameters)
  : InterfaceDiffusionBase(parameters)
{
}

Real
InterfaceDiffusionFluxMatch::computeQpResidual(Moose::DGResidualType type)
{
  // equal gradients means difference is zero
  Real res =
      _D * _grad_u[_qp] * _normals[_qp] - _D_neighbor * _grad_neighbor_value[_qp] * _normals[_qp];

  switch (type)
  {
    case Moose::Element:
      return res * _test[_i][_qp];

    case Moose::Neighbor:
      return res * _test_neighbor[_i][_qp];
  }

  mooseError("Internal error.");
}

Real
InterfaceDiffusionFluxMatch::computeQpJacobian(Moose::DGJacobianType type)
{
  switch (type)
  {
    case Moose::ElementElement:
      return _D * _grad_phi[_j][_qp] * _normals[_qp] * _test[_i][_qp];

    case Moose::NeighborNeighbor:
      return -_D_neighbor * _grad_phi_neighbor[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp];

    case Moose::ElementNeighbor:
      return -_D_neighbor * _grad_phi_neighbor[_j][_qp] * _normals[_qp] * _test[_i][_qp];

    case Moose::NeighborElement:
      return _D * _grad_phi[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp];
  }
  mooseError("Internal error.");
}
