//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyInterfaceDiffusionDot.h"

registerMooseObject("MooseTestApp", PenaltyInterfaceDiffusionDot);

InputParameters
PenaltyInterfaceDiffusionDot::validParams()
{
  InputParameters params = InterfaceTimeKernel::validParams();
  params.addRequiredParam<Real>(
      "penalty", "The penalty that penalizes jump between primary and neighbor variables.");
  return params;
}

PenaltyInterfaceDiffusionDot::PenaltyInterfaceDiffusionDot(const InputParameters & parameters)
  : InterfaceTimeKernel(parameters), _penalty(getParam<Real>("penalty"))
{
}

Real
PenaltyInterfaceDiffusionDot::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;

  switch (type)
  {
    case Moose::Element:
      r = _test[_i][_qp] * _penalty * (_u_dot[_qp] - _neighbor_value_dot[_qp]);
      break;

    case Moose::Neighbor:
      r = _test_neighbor[_i][_qp] * -_penalty * (_u_dot[_qp] - _neighbor_value_dot[_qp]);
      break;
  }

  return r;
}

Real
PenaltyInterfaceDiffusionDot::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0;

  switch (type)
  {

    case Moose::ElementElement:
      jac = _test[_i][_qp] * _penalty * _phi[_j][_qp] * _du_dot_du[_qp];
      break;

    case Moose::ElementNeighbor:
      jac = _test[_i][_qp] * _penalty * -_phi_neighbor[_j][_qp] * _dneighbor_value_dot_du[_qp];
      break;

    case Moose::NeighborElement:
      jac = _test_neighbor[_i][_qp] * -_penalty * _phi[_j][_qp] * _du_dot_du[_qp];
      break;

    case Moose::NeighborNeighbor:
      jac = _test_neighbor[_i][_qp] * -_penalty * -_phi_neighbor[_j][_qp] *
            _dneighbor_value_dot_du[_qp];
      break;
  }

  return jac;
}
