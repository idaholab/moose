/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "OneSideDiffusion.h"

#include <cmath>

template <>
InputParameters
validParams<OneSideDiffusion>()
{
  InputParameters params = validParams<InterfaceKernel>();
  params.addParam<Real>("D", 1., "The diffusion coefficient.");
  return params;
}

OneSideDiffusion::OneSideDiffusion(const InputParameters & parameters)
  : InterfaceKernel(parameters), _D(getParam<Real>("D"))
{
  if (!parameters.isParamValid("boundary"))
  {
    mooseError("In order to use the OneSideDiffusion dgkernel, you must specify a boundary where "
               "it will live.");
  }
}

Real
OneSideDiffusion::computeQpResidual(Moose::DGResidualType type)
{
  Real r = -_D * _grad_u[_qp] * _normals[_qp];

  switch (type)
  {
    case Moose::Element:
      r *= 0;
      break;

    case Moose::Neighbor:
      r *= -_test_neighbor[_i][_qp];
      break;
  }

  return r;
}

Real
OneSideDiffusion::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0;

  switch (type)
  {

    case Moose::ElementElement:
      jac -= 0;
      break;

    case Moose::NeighborNeighbor:
      jac += 0;
      break;

    case Moose::NeighborElement:
      jac += _test_neighbor[_i][_qp] * _D * _grad_phi[_j][_qp] * _normals[_qp];
      break;

    case Moose::ElementNeighbor:
      jac -= 0;
      break;
  }

  return jac;
}
