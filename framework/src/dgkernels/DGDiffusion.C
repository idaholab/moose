//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DGDiffusion.h"
#include "MooseVariableFE.h"

#include "libmesh/utility.h"

registerMooseObject("MooseApp", DGDiffusion);

InputParameters
DGDiffusion::validParams()
{
  InputParameters params = DGKernel::validParams();
  params.addClassDescription("Computes residual contribution for the diffusion operator using "
                             "discontinous Galerkin method.");
  // See header file for sigma and epsilon
  params.addRequiredParam<Real>("sigma", "sigma");
  params.addRequiredParam<Real>("epsilon", "epsilon");
  params.addParam<MaterialPropertyName>(
      "diff", 1., "The diffusion (or thermal conductivity or viscosity) coefficient.");
  return params;
}

DGDiffusion::DGDiffusion(const InputParameters & parameters)
  : DGKernel(parameters),
    _epsilon(getParam<Real>("epsilon")),
    _sigma(getParam<Real>("sigma")),
    _diff(getMaterialProperty<Real>("diff")),
    _diff_neighbor(getNeighborMaterialProperty<Real>("diff"))
{
}

Real
DGDiffusion::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0.0;

  const int elem_b_order = std::max(libMesh::Order(1), _var.order());
  const Real h_elem =
      _current_elem_volume / _current_side_volume * 1.0 / Utility::pow<2>(elem_b_order);

  switch (type)
  {
    case Moose::Element:
      r -= 0.5 *
           (_diff[_qp] * _grad_u[_qp] * _normals[_qp] +
            _diff_neighbor[_qp] * _grad_u_neighbor[_qp] * _normals[_qp]) *
           _test[_i][_qp];
      r += _epsilon * 0.5 * (_u[_qp] - _u_neighbor[_qp]) * _diff[_qp] * _grad_test[_i][_qp] *
           _normals[_qp];
      r += _sigma / h_elem * (_u[_qp] - _u_neighbor[_qp]) * _test[_i][_qp];
      break;

    case Moose::Neighbor:
      r += 0.5 *
           (_diff[_qp] * _grad_u[_qp] * _normals[_qp] +
            _diff_neighbor[_qp] * _grad_u_neighbor[_qp] * _normals[_qp]) *
           _test_neighbor[_i][_qp];
      r += _epsilon * 0.5 * (_u[_qp] - _u_neighbor[_qp]) * _diff_neighbor[_qp] *
           _grad_test_neighbor[_i][_qp] * _normals[_qp];
      r -= _sigma / h_elem * (_u[_qp] - _u_neighbor[_qp]) * _test_neighbor[_i][_qp];
      break;
  }

  return r;
}

Real
DGDiffusion::computeQpJacobian(Moose::DGJacobianType type)
{
  Real r = 0.0;

  const int elem_b_order = std::max(libMesh::Order(1), _var.order());
  const Real h_elem =
      _current_elem_volume / _current_side_volume * 1.0 / Utility::pow<2>(elem_b_order);

  switch (type)
  {
    case Moose::ElementElement:
      r -= 0.5 * _diff[_qp] * _grad_phi[_j][_qp] * _normals[_qp] * _test[_i][_qp];
      r += _epsilon * 0.5 * _phi[_j][_qp] * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp];
      r += _sigma / h_elem * _phi[_j][_qp] * _test[_i][_qp];
      break;

    case Moose::ElementNeighbor:
      r -= 0.5 * _diff_neighbor[_qp] * _grad_phi_neighbor[_j][_qp] * _normals[_qp] * _test[_i][_qp];
      r += _epsilon * 0.5 * -_phi_neighbor[_j][_qp] * _diff[_qp] * _grad_test[_i][_qp] *
           _normals[_qp];
      r += _sigma / h_elem * -_phi_neighbor[_j][_qp] * _test[_i][_qp];
      break;

    case Moose::NeighborElement:
      r += 0.5 * _diff[_qp] * _grad_phi[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp];
      r += _epsilon * 0.5 * _phi[_j][_qp] * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] *
           _normals[_qp];
      r -= _sigma / h_elem * _phi[_j][_qp] * _test_neighbor[_i][_qp];
      break;

    case Moose::NeighborNeighbor:
      r += 0.5 * _diff_neighbor[_qp] * _grad_phi_neighbor[_j][_qp] * _normals[_qp] *
           _test_neighbor[_i][_qp];
      r += _epsilon * 0.5 * -_phi_neighbor[_j][_qp] * _diff_neighbor[_qp] *
           _grad_test_neighbor[_i][_qp] * _normals[_qp];
      r -= _sigma / h_elem * -_phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
      break;
  }

  return r;
}
