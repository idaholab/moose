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

#include "DGCoupledDiffusion.h"

// MOOSE includes
#include "MooseVariable.h"

template <>
InputParameters
validParams<DGCoupledDiffusion>()
{
  InputParameters params = validParams<DGKernel>();
  params.addRequiredCoupledVar("v", "The governing variable that controls diffusion of u.");
  return params;
}

DGCoupledDiffusion::DGCoupledDiffusion(const InputParameters & parameters)
  : DGKernel(parameters),
    _v_var(*getVar("v", 0)),
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
