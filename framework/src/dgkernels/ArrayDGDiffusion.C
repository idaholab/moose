//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayDGDiffusion.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/utility.h"

registerMooseObject("MooseApp", ArrayDGDiffusion);

InputParameters
ArrayDGDiffusion::validParams()
{
  InputParameters params = ArrayDGKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "diff", "The diffusion (or thermal conductivity or viscosity) coefficient.");
  params.addParam<Real>("sigma", 4, "sigma");
  params.addParam<Real>("epsilon", 1, "epsilon");
  params.addClassDescription("Implements interior penalty method for array diffusion equations.");
  return params;
}

ArrayDGDiffusion::ArrayDGDiffusion(const InputParameters & parameters)
  : ArrayDGKernel(parameters),
    _epsilon(getParam<Real>("epsilon")),
    _sigma(getParam<Real>("sigma")),
    _diff(getMaterialProperty<RealEigenVector>("diff")),
    _diff_neighbor(getNeighborMaterialProperty<RealEigenVector>("diff")),
    _res1(_count),
    _res2(_count)
{
}

void
ArrayDGDiffusion::initQpResidual(Moose::DGResidualType type)
{
  mooseAssert(_diff[_qp].size() == _count && _diff_neighbor[_qp].size() == _count,
              "'diff' size is inconsistent with the number of components of array "
              "variable");

  const int elem_b_order = std::max(libMesh::Order(1), _var.order());
  const Real h_elem =
      _current_elem_volume / _current_side_volume * 1.0 / Utility::pow<2>(elem_b_order);

  // WARNING: use noalias() syntax with caution. See ArrayDiffusion.C for more details.
  _res1.noalias() = _diff[_qp].asDiagonal() * _grad_u[_qp] * _array_normals[_qp];
  _res1.noalias() += _diff_neighbor[_qp].asDiagonal() * _grad_u_neighbor[_qp] * _array_normals[_qp];
  _res1 *= 0.5;
  _res1 -= (_u[_qp] - _u_neighbor[_qp]) * _sigma / h_elem;

  switch (type)
  {
    case Moose::Element:
      _res2.noalias() = _diff[_qp].asDiagonal() * (_u[_qp] - _u_neighbor[_qp]) * _epsilon * 0.5;
      break;

    case Moose::Neighbor:
      _res2.noalias() =
          _diff_neighbor[_qp].asDiagonal() * (_u[_qp] - _u_neighbor[_qp]) * _epsilon * 0.5;
      break;
  }
}

void
ArrayDGDiffusion::computeQpResidual(Moose::DGResidualType type, RealEigenVector & residual)
{
  switch (type)
  {
    case Moose::Element:
      residual = -_res1 * _test[_i][_qp] + _res2 * (_grad_test[_i][_qp] * _normals[_qp]);
      break;

    case Moose::Neighbor:
      residual =
          _res1 * _test_neighbor[_i][_qp] + _res2 * (_grad_test_neighbor[_i][_qp] * _normals[_qp]);
      break;
  }
}

RealEigenVector
ArrayDGDiffusion::computeQpJacobian(Moose::DGJacobianType type)
{
  RealEigenVector r = RealEigenVector::Zero(_count);

  const int elem_b_order = std::max(libMesh::Order(1), _var.order());
  const Real h_elem =
      _current_elem_volume / _current_side_volume * 1.0 / Utility::pow<2>(elem_b_order);

  switch (type)
  {
    case Moose::ElementElement:
      r -= _grad_phi[_j][_qp] * _normals[_qp] * _test[_i][_qp] * 0.5 * _diff[_qp];
      r += _grad_test[_i][_qp] * _normals[_qp] * _epsilon * 0.5 * _phi[_j][_qp] * _diff[_qp];
      r += RealEigenVector::Constant(_count, _sigma / h_elem * _phi[_j][_qp] * _test[_i][_qp]);
      break;

    case Moose::ElementNeighbor:
      r -= _grad_phi_neighbor[_j][_qp] * _normals[_qp] * _test[_i][_qp] * 0.5 * _diff_neighbor[_qp];
      r -= _grad_test[_i][_qp] * _normals[_qp] * _epsilon * 0.5 * _phi_neighbor[_j][_qp] *
           _diff[_qp];
      r -= RealEigenVector::Constant(_count,
                                     _sigma / h_elem * _phi_neighbor[_j][_qp] * _test[_i][_qp]);
      break;

    case Moose::NeighborElement:
      r += _grad_phi[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp] * 0.5 * _diff[_qp];
      r += _grad_test_neighbor[_i][_qp] * _normals[_qp] * _epsilon * 0.5 * _phi[_j][_qp] *
           _diff_neighbor[_qp];
      r -= RealEigenVector::Constant(_count,
                                     _sigma / h_elem * _phi[_j][_qp] * _test_neighbor[_i][_qp]);
      break;

    case Moose::NeighborNeighbor:
      r += _grad_phi_neighbor[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp] * 0.5 *
           _diff_neighbor[_qp];
      r -= _grad_test_neighbor[_i][_qp] * _normals[_qp] * _epsilon * 0.5 * _phi_neighbor[_j][_qp] *
           _diff_neighbor[_qp];
      r += RealEigenVector::Constant(
          _count, _sigma / h_elem * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp]);
      break;
  }

  return r;
}
