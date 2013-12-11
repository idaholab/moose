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

#include "DGMatDiffusion.h"

#include <cmath>

template<>
InputParameters validParams<DGMatDiffusion>()
{
  InputParameters params = validParams<DGKernel>();
  // See header file for sigma and epsilon
  params.addRequiredParam<Real>("sigma", "sigma");
  params.addRequiredParam<Real>("epsilon", "epsilon");
  params.addRequiredParam<std::string>("prop_name", "diff1");
  return params;
}

DGMatDiffusion::DGMatDiffusion(const std::string & name, InputParameters parameters)
  :DGKernel(name, parameters),
   _epsilon(getParam<Real>("epsilon")),
   _sigma(getParam<Real>("sigma")),
   _prop_name(getParam<std::string>("prop_name")),
   _diff(getMaterialProperty<Real>(_prop_name)),
   _diff_neighbor(getNeighborMaterialProperty<Real>(_prop_name))
{
}

Real
DGMatDiffusion::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;

  const unsigned int elem_b_order = static_cast<unsigned int> (_var.getOrder());
  const double h_elem = _current_elem->volume()/_current_side_elem->volume() * 1./std::pow(elem_b_order, 2.);

  switch (type)
  {
  case Moose::Element:
    r -= 0.5 * (_diff[_qp] * _grad_u[_qp] * _normals[_qp] + _diff_neighbor[_qp] * _grad_u_neighbor[_qp] * _normals[_qp]) * _test[_i][_qp];
    r += _epsilon * 0.5 * (_u[_qp] - _u_neighbor[_qp]) * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp];
    r += _sigma / h_elem * (_u[_qp] - _u_neighbor[_qp]) * _test[_i][_qp];
    break;

  case Moose::Neighbor:
    r += 0.5 * (_diff[_qp] * _grad_u[_qp] * _normals[_qp] + _diff_neighbor[_qp] * _grad_u_neighbor[_qp] * _normals[_qp]) * _test_neighbor[_i][_qp];
    r += _epsilon * 0.5 * (_u[_qp] - _u_neighbor[_qp]) * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp];
    r -= _sigma / h_elem * (_u[_qp] - _u_neighbor[_qp]) * _test_neighbor[_i][_qp];
    break;
  }

  return r;
}

Real
DGMatDiffusion::computeQpJacobian(Moose::DGJacobianType type)
{
  Real r = 0;

  const unsigned int elem_b_order = static_cast<unsigned int> (_var.getOrder());
  const double h_elem = _current_elem->volume()/_current_side_elem->volume() * 1./std::pow(elem_b_order, 2.);

  switch (type)
  {
  case Moose::ElementElement:
    r -= 0.5 * _diff[_qp] * _grad_test[_j][_qp] * _normals[_qp] * _test[_i][_qp];
    r += _epsilon * 0.5 * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _test[_j][_qp];
    r += _sigma / h_elem * _test[_j][_qp] * _test[_i][_qp];
    break;

  case Moose::ElementNeighbor:
    r += 0.5 * _diff[_qp] * _grad_test[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp];
    r += _epsilon * 0.5 * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _test[_j][_qp];
    r -= _sigma / h_elem * _test[_j][_qp] * _test_neighbor[_i][_qp];
    break;

  case Moose::NeighborElement:
    r -= 0.5 * _diff_neighbor[_qp] * _grad_test_neighbor[_j][_qp] * _normals[_qp] * _test[_i][_qp];
    r -= _epsilon * 0.5 * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp] * _test_neighbor[_j][_qp];
    r -= _sigma / h_elem * _test_neighbor[_j][_qp] * _test[_i][_qp];
    break;

  case Moose::NeighborNeighbor:
    r += 0.5 * _diff_neighbor[_qp] * _grad_test_neighbor[_j][_qp] * _normals[_qp] * _test_neighbor[_i][_qp];
    r -= _epsilon * 0.5 * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] * _normals[_qp] * _test_neighbor[_j][_qp];
    r += _sigma / h_elem * _test_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
    break;
  }

  return r;
}
