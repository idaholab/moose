//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneSideDiffusion.h"

#include <cmath>

registerMooseObject("MooseTestApp", OneSideDiffusion);

InputParameters
OneSideDiffusion::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
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
