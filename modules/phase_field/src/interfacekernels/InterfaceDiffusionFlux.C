/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "InterfaceDiffusionFlux.h"

template<>
InputParameters validParams<InterfaceDiffusionFlux>()
{
  InputParameters params = validParams<InterfaceKernel>();
  params.addClassDescription("Add weak form surface terms of the Diffusion equation for two different variables across a subdomain boundary");
  params.addParam<Real>("D", 1.0, "Diffusion coefficient");
  params.addParam<Real>("D_neighbor", 1.0, "Neighbor variable diffusion coefficient");
  return params;
}

InterfaceDiffusionFlux::InterfaceDiffusionFlux(const InputParameters & parameters) :
    InterfaceKernel(parameters),
    _D(getParam<Real>("D")),
    _D_neighbor(getParam<Real>("D_neighbor"))
{
}

Real
InterfaceDiffusionFlux::computeQpResidual(Moose::DGResidualType type)
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
InterfaceDiffusionFlux::computeQpJacobian(Moose::DGJacobianType type)
{
  switch (type)
  {
    case Moose::ElementElement:
      return -_D * _grad_phi[_j][_qp] * _normals[_qp]* _test[_i][_qp];

    case Moose::NeighborNeighbor:
      return -_D_neighbor * _grad_phi_neighbor[_j][_qp] * -_normals[_qp] * _test_neighbor[_i][_qp];

    case Moose::ElementNeighbor:
      return 0.0;

    case Moose::NeighborElement:
      return 0.0;
  }

  mooseError("Internal error.");
}
