//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DGCoupledDiffusion.h"

// MOOSE includes
#include "MooseVariableFE.h"

registerMooseObject("MooseTestApp", DGCoupledDiffusion);

InputParameters
DGCoupledDiffusion::validParams()
{
  InputParameters params = DGKernel::validParams();
  params.addRequiredCoupledVar("v", "The governing variable that controls diffusion of u.");
  return params;
}

DGCoupledDiffusion::DGCoupledDiffusion(const InputParameters & parameters)
  : DGKernel(parameters),
    _v_var(dynamic_cast<MooseVariable &>(*getVar("v", 0))),
    _v(_v_var.sln()),
    _v_neighbor(_v_var.slnNeighbor()),
    _grad_v(_v_var.gradSln()),
    _grad_v_neighbor(_v_var.gradSlnNeighbor()),
    _v_id(coupled("v"))
{
}

Real
DGCoupledDiffusion::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;

  switch (type)
  {
    case Moose::Element:
      r -= _normals[_qp] * (_grad_v[_qp] - _grad_v_neighbor[_qp]) * _test[_i][_qp] +
           (_v[_qp] - _v_neighbor[_qp]) * _grad_test[_i][_qp] * _normals[_qp];
      break;

    case Moose::Neighbor:
      r += _normals[_qp] * (_grad_v[_qp] - _grad_v_neighbor[_qp]) * _test[_i][_qp] +
           (_v[_qp] - _v_neighbor[_qp]) * _grad_test[_i][_qp] * _normals[_qp];
      break;
  }

  return r;
}

Real DGCoupledDiffusion::computeQpJacobian(Moose::DGJacobianType /*type*/) { return 0; }

Real
DGCoupledDiffusion::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  Real r = 0;

  if (jvar == _v_id)
  {
    switch (type)
    {
      case Moose::ElementElement:
        r -= _normals[_qp] * (_grad_phi[_j][_qp]) * _test[_i][_qp] +
             (_phi[_j][_qp]) * _grad_test[_i][_qp] * _normals[_qp];
        break;

      case Moose::ElementNeighbor:
        r -= _normals[_qp] * (-_grad_phi_neighbor[_j][_qp]) * _test[_i][_qp] +
             (-_phi_neighbor[_j][_qp]) * _grad_test[_i][_qp] * _normals[_qp];
        break;

      case Moose::NeighborElement:
        r += _normals[_qp] * (_grad_phi[_j][_qp]) * _test[_i][_qp] +
             (_phi[_j][_qp]) * _grad_test[_i][_qp] * _normals[_qp];
        break;

      case Moose::NeighborNeighbor:
        r += _normals[_qp] * (-_grad_phi_neighbor[_j][_qp]) * _test[_i][_qp] +
             (-_phi_neighbor[_j][_qp]) * _grad_test[_i][_qp] * _normals[_qp];
        break;
    }
  }

  return r;
}
