//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyInterfaceDiffusion.h"

registerMooseObject("MooseTestApp", PenaltyInterfaceDiffusion);

InputParameters
PenaltyInterfaceDiffusion::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addRequiredParam<Real>(
      "penalty", "The penalty that penalizes jump between primary and neighbor variables.");
  params.addParam<MaterialPropertyName>(
      "jump_prop_name", "the name of the material property that calculates the jump.");
  return params;
}

PenaltyInterfaceDiffusion::PenaltyInterfaceDiffusion(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _penalty(getParam<Real>("penalty")),
    _jump(isParamValid("jump_prop_name") ? &getMaterialProperty<Real>("jump_prop_name") : nullptr)
{
}

Real
PenaltyInterfaceDiffusion::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;

  Real jump_value = 0;

  if (_jump != nullptr)
  {
    jump_value = (*_jump)[_qp];
    mooseAssert(jump_value == _u[_qp] - _neighbor_value[_qp], "");
  }
  else
    jump_value = _u[_qp] - _neighbor_value[_qp];

  switch (type)
  {
    case Moose::Element:
      r = _test[_i][_qp] * _penalty * jump_value;
      break;

    case Moose::Neighbor:
      r = _test_neighbor[_i][_qp] * -_penalty * jump_value;
      break;
  }

  return r;
}

Real
PenaltyInterfaceDiffusion::computeQpJacobian(Moose::DGJacobianType type)
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

    case Moose::NeighborElement:
      jac = _test_neighbor[_i][_qp] * -_penalty * _phi[_j][_qp];
      break;

    case Moose::NeighborNeighbor:
      jac = _test_neighbor[_i][_qp] * -_penalty * -_phi_neighbor[_j][_qp];
      break;
  }

  return jac;
}
