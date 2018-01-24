/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "InterfaceDiffusionBoundaryTerm.h"

template <>
InputParameters
validParams<InterfaceDiffusionBoundaryTerm>()
{
  InputParameters params = validParams<InterfaceDiffusionBase>();
  params.addClassDescription("Add weak form surface terms of the Diffusion equation for two "
                             "different variables across a subdomain boundary");
  return params;
}

InterfaceDiffusionBoundaryTerm::InterfaceDiffusionBoundaryTerm(const InputParameters & parameters)
  : InterfaceDiffusionBase(parameters)
{
}

Real
InterfaceDiffusionBoundaryTerm::computeQpResidual(Moose::DGResidualType type)
{
  // add weak form surface terms for the diffusion equation
  switch (type)
  {
    case Moose::Element:
      return -_D * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp];

    case Moose::Neighbor:
      return -_D_neighbor * _grad_neighbor_value[_qp] * -_normals[_qp] * _test_neighbor[_i][_qp];
  }

  mooseError("Internal error.");
}

Real
InterfaceDiffusionBoundaryTerm::computeQpJacobian(Moose::DGJacobianType type)
{
  switch (type)
  {
    case Moose::ElementElement:
      return -_D * _grad_phi[_j][_qp] * _normals[_qp] * _test[_i][_qp];

    case Moose::NeighborNeighbor:
      return -_D_neighbor * _grad_phi_neighbor[_j][_qp] * -_normals[_qp] * _test_neighbor[_i][_qp];

    case Moose::ElementNeighbor:
      return 0.0;

    case Moose::NeighborElement:
      return 0.0;
  }

  mooseError("Internal error.");
}
