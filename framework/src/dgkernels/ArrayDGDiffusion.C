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

template <>
InputParameters
validParams<ArrayDGDiffusion>()
{
  InputParameters params = validParams<ArrayDGKernel>();
  params.addRequiredParam<MaterialPropertyName>(
      "diff", "The diffusion (or thermal conductivity or viscosity) coefficient.");
  params.addParam<Real>("sigma", 4, "sigma");
  params.addParam<Real>("epsilon", 1, "epsilon");
  return params;
}

ArrayDGDiffusion::ArrayDGDiffusion(const InputParameters & parameters)
  : ArrayDGKernel(parameters),
    _epsilon(getParam<Real>("epsilon")),
    _sigma(getParam<Real>("sigma")),
    _diff(getMaterialProperty<RealArrayValue>("diff")),
    _diff_neighbor(getNeighborMaterialProperty<RealArrayValue>("diff"))
{
}

RealArrayValue
ArrayDGDiffusion::computeQpResidual(Moose::DGResidualType type)
{
  RealArrayValue r = RealArrayValue::Zero(_count);

  const unsigned int elem_b_order = _var.order();
  const double h_elem =
      _current_elem->volume() / _current_side_elem->volume() * 1. / Utility::pow<2>(elem_b_order);

  switch (type)
  {
    case Moose::Element:
      r -= (_diff[_qp].cwiseProduct(_grad_u[_qp] * _array_normals[_qp]) +
            _diff_neighbor[_qp].cwiseProduct(_grad_u_neighbor[_qp] * _array_normals[_qp])) *
           (0.5 * _test[_i][_qp]);
      r += _diff[_qp].cwiseProduct(_u[_qp] - _u_neighbor[_qp]) * (_epsilon * 0.5) *
           (_grad_test[_i][_qp] * _normals[_qp]);
      r += (_u[_qp] - _u_neighbor[_qp]) * (_sigma / h_elem * _test[_i][_qp]);
      break;

    case Moose::Neighbor:
      r += (_diff[_qp].cwiseProduct(_grad_u[_qp] * _array_normals[_qp]) +
            _diff_neighbor[_qp].cwiseProduct(_grad_u_neighbor[_qp] * _array_normals[_qp])) *
           (0.5 * _test_neighbor[_i][_qp]);
      r += _diff_neighbor[_qp].cwiseProduct(_u[_qp] - _u_neighbor[_qp]) * (_epsilon * 0.5) *
           (_grad_test_neighbor[_i][_qp] * _normals[_qp]);
      r -= (_u[_qp] - _u_neighbor[_qp]) * (_sigma / h_elem * _test_neighbor[_i][_qp]);
      break;
  }

  return r;
}

RealArrayValue
ArrayDGDiffusion::computeQpJacobian(Moose::DGJacobianType type)
{
  RealArrayValue r = RealArrayValue::Zero(_count);

  const unsigned int elem_b_order = _var.order();
  const double h_elem =
      _current_elem->volume() / _current_side_elem->volume() * 1. / Utility::pow<2>(elem_b_order);

  switch (type)
  {
    case Moose::ElementElement:
      r -= _grad_phi[_j][_qp] * _normals[_qp] * _test[_i][_qp] * 0.5 * _diff[_qp];
      r += _grad_test[_i][_qp] * _normals[_qp] * _epsilon * 0.5 * _phi[_j][_qp] * _diff[_qp];
      r += RealArrayValue::Constant(_count, _sigma / h_elem * _phi[_j][_qp] * _test[_i][_qp]);
      break;

    case Moose::ElementNeighbor:
      r -= _grad_phi_neighbor[_j][_qp] * _normals[_qp] * _test[_i][_qp] * 0.5 * _diff_neighbor[_qp];
      r -= _grad_test[_i][_qp] * _normals[_qp] * _epsilon * 0.5 * _phi_neighbor[_j][_qp] *
           _diff[_qp];
      r -= RealArrayValue::Constant(_count,
                                    _sigma / h_elem * _phi_neighbor[_j][_qp] * _test[_i][_qp]);
      break;

    case Moose::NeighborElement:
      r += _grad_phi[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp] * 0.5 * _diff[_qp];
      r += _grad_test_neighbor[_i][_qp] * _normals[_qp] * _epsilon * 0.5 * _phi[_j][_qp] *
           _diff_neighbor[_qp];
      r -= RealArrayValue::Constant(_count,
                                    _sigma / h_elem * _phi[_j][_qp] * _test_neighbor[_i][_qp]);
      break;

    case Moose::NeighborNeighbor:
      r += _grad_phi_neighbor[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp] * 0.5 *
           _diff_neighbor[_qp];
      r -= _grad_test_neighbor[_i][_qp] * _normals[_qp] * _epsilon * 0.5 * _phi_neighbor[_j][_qp] *
           _diff_neighbor[_qp];
      r += RealArrayValue::Constant(
          _count, _sigma / h_elem * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp]);
      break;
  }

  return r;
}
