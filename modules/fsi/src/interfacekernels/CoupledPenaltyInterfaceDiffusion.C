//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledPenaltyInterfaceDiffusion.h"

registerMooseObject("FsiApp", CoupledPenaltyInterfaceDiffusion);

InputParameters
CoupledPenaltyInterfaceDiffusion::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addClassDescription(
      "Enforces continuity of flux and continuity of solution via penalty across an interface.");
  params.addRequiredParam<Real>(
      "penalty",
      "The penalty that penalizes jump between primary and neighbor secondary variables.");
  params.addCoupledVar("primary_coupled_var", "The coupled variable on the master side");
  params.addCoupledVar("secondary_coupled_var", "The coupled variable on the slave side");
  return params;
}

CoupledPenaltyInterfaceDiffusion::CoupledPenaltyInterfaceDiffusion(
    const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _penalty(getParam<Real>("penalty")),
    _primary_coupled_value(isCoupled("primary_coupled_var") ? coupledValue("primary_coupled_var")
                                                            : _var.sln()),
    _secondary_coupled_value(isCoupled("secondary_coupled_var")
                                 ? coupledNeighborValue("secondary_coupled_var")
                                 : _neighbor_var.slnNeighbor()),
    _primary_coupled_id(isCoupled("primary_coupled_var") ? coupled("primary_coupled_var")
                                                         : _var.number()),
    _secondary_coupled_id(isCoupled("secondary_coupled_var") ? coupled("secondary_coupled_var")
                                                             : _neighbor_var.number())
{
}

Real
CoupledPenaltyInterfaceDiffusion::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;

  switch (type)
  {
    case Moose::Element:
      r = _test[_i][_qp] * _penalty * (_primary_coupled_value[_qp] - _secondary_coupled_value[_qp]);
      break;

    case Moose::Neighbor:
      r = _test_neighbor[_i][_qp] * -_penalty *
          (_primary_coupled_value[_qp] - _secondary_coupled_value[_qp]);
      break;
  }

  return r;
}

Real CoupledPenaltyInterfaceDiffusion::computeQpJacobian(Moose::DGJacobianType) { return 0; }

Real
CoupledPenaltyInterfaceDiffusion::computeQpOffDiagJacobian(Moose::DGJacobianType type,
                                                           unsigned int jvar)
{
  if (jvar == _primary_coupled_id)
  {
    switch (type)
    {
      case Moose::ElementElement:
        return _test[_i][_qp] * _penalty * _phi[_j][_qp];
      case Moose::NeighborElement:
        return _test_neighbor[_i][_qp] * -_penalty * _phi[_j][_qp];
      case Moose::ElementNeighbor:
      case Moose::NeighborNeighbor:
        break;
    }
  }
  else if (jvar == _secondary_coupled_id)
  {
    switch (type)
    {
      case Moose::ElementNeighbor:
        return _test[_i][_qp] * _penalty * -_phi_neighbor[_j][_qp];
      case Moose::NeighborNeighbor:
        return _test_neighbor[_i][_qp] * -_penalty * -_phi_neighbor[_j][_qp];
      case Moose::ElementElement:
      case Moose::NeighborElement:
        break;
    }
  }
  return 0;
}

void
CoupledPenaltyInterfaceDiffusion::computeElementOffDiagJacobian(unsigned int jvar)
{
  computeOffDiagElemNeighJacobian(Moose::ElementElement, jvar);
  computeOffDiagElemNeighJacobian(Moose::ElementNeighbor, jvar);
}

void
CoupledPenaltyInterfaceDiffusion::computeNeighborOffDiagJacobian(unsigned int jvar)
{
  computeOffDiagElemNeighJacobian(Moose::NeighborElement, jvar);
  computeOffDiagElemNeighJacobian(Moose::NeighborNeighbor, jvar);
}
