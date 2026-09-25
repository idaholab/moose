//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleCohesiveFlux.h"

registerMooseObject("MooseTestApp", SimpleCohesiveFlux);

InputParameters
SimpleCohesiveFlux::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addClassDescription(
      "Applies a scalar cohesive flux supplied as a material property and uses the material "
      "property derivative for the Jacobian.");
  params.addRequiredParam<MaterialPropertyName>("flux", "The scalar cohesive flux.");
  params.addRequiredParam<MaterialPropertyName>(
      "dflux_djump", "The derivative of the scalar cohesive flux with respect to the jump.");
  return params;
}

SimpleCohesiveFlux::SimpleCohesiveFlux(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _q(getMaterialProperty<Real>("flux")),
    _dq_djump(getMaterialProperty<Real>("dflux_djump"))
{
}

Real
SimpleCohesiveFlux::computeQpResidual(Moose::DGResidualType type)
{
  switch (type)
  {
    case Moose::Element:
      return _test[_i][_qp] * _q[_qp];

    case Moose::Neighbor:
      return -_test_neighbor[_i][_qp] * _q[_qp];
  }

  mooseError("Unknown DG residual type.");
}

Real
SimpleCohesiveFlux::computeQpJacobian(Moose::DGJacobianType type)
{
  switch (type)
  {
    case Moose::ElementElement:
      return _test[_i][_qp] * _dq_djump[_qp] * _phi[_j][_qp];

    case Moose::ElementNeighbor:
      return -_test[_i][_qp] * _dq_djump[_qp] * _phi_neighbor[_j][_qp];

    case Moose::NeighborElement:
      return -_test_neighbor[_i][_qp] * _dq_djump[_qp] * _phi[_j][_qp];

    case Moose::NeighborNeighbor:
      return _test_neighbor[_i][_qp] * _dq_djump[_qp] * _phi_neighbor[_j][_qp];
  }

  mooseError("Unknown DG Jacobian type.");
}
