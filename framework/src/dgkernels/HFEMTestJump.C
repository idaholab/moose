//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HFEMTestJump.h"

registerMooseObject("MooseApp", HFEMTestJump);

InputParameters
HFEMTestJump::validParams()
{
  InputParameters params = DGKernel::validParams();
  params.addRequiredCoupledVar("side_variable", "side variable to use as Lagrange multiplier");
  params.addClassDescription("Imposes constraints for HFEM with side-discontinuous variables.");
  return params;
}

HFEMTestJump::HFEMTestJump(const InputParameters & parameters)
  : DGKernel(parameters),
    _lambda_var(*getVar("side_variable", 0)),
    _lambda(_is_implicit ? _lambda_var.sln() : _lambda_var.slnOld()),
    _phi_lambda(_lambda_var.phiFace()),
    _test_lambda(_lambda_var.phiFace()),
    _lambda_id(coupled("side_variable"))
{
}

Real
HFEMTestJump::computeQpResidual(Moose::DGResidualType type)
{
  switch (type)
  {
    case Moose::Element:
      return -_lambda[_qp] * _test[_i][_qp];
      break;

    case Moose::Neighbor:
      return _lambda[_qp] * _test_neighbor[_i][_qp];
      break;
  }
  return 0;
}

Real
HFEMTestJump::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  if (jvar != _lambda_id)
    return 0;

  switch (type)
  {
    case Moose::ElementElement:
      return -_phi_lambda[_j][_qp] * _test[_i][_qp];

    case Moose::NeighborElement:
      return _phi_lambda[_j][_qp] * _test_neighbor[_i][_qp];

    case Moose::ElementNeighbor:
      return 0;

    case Moose::NeighborNeighbor:
      return 0;

    default:
      break;
  }

  return 0;
}
