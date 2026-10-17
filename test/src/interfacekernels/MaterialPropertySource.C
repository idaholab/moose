//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialPropertySource.h"

registerMooseObject("MooseTestApp", MaterialPropertySource);

InputParameters
MaterialPropertySource::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("source", "The source.");
  params.addRequiredParam<MaterialPropertyName>(
      "dsource_du", "The derivative of the source with respect to the solution variable.");
  params.addRequiredParam<MaterialPropertyName>(
      "dsource_du_neighbor",
      "The derivative of the source with respect to the neighbor solution variable.");
  return params;
}

MaterialPropertySource::MaterialPropertySource(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _source(getMaterialProperty<Real>("source")),
    _dsource_du(getMaterialProperty<Real>("dsource_du")),
    _dsource_du_neigh(getMaterialProperty<Real>("dsource_du_neighbor")),
    _source_neigh(getNeighborMaterialProperty<Real>("source")),
    _dsource_neigh_du(getNeighborMaterialProperty<Real>("dsource_du_neighbor")),
    _dsource_neigh_du_neigh(getNeighborMaterialProperty<Real>("dsource_du"))
{
}

Real
MaterialPropertySource::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;

  switch (type)
  {
    case Moose::Element:
      r = -_test[_i][_qp] * (1.1 * _source[_qp] + 0.5 * _source_neigh[_qp]);
      break;

    case Moose::Neighbor:
      r = -_test_neighbor[_i][_qp] * (0.6 * _source[_qp] + 0.8 * _source_neigh[_qp]);
      break;
  }

  return r;
}

Real
MaterialPropertySource::computeQpJacobian(Moose::DGJacobianType type)
{
  Real J = 0;

  switch (type)
  {
    case Moose::ElementElement:
      J = -_test[_i][_qp] * (1.1 * _dsource_du[_qp] + 0.5 * _dsource_neigh_du[_qp]) * _phi[_j][_qp];
      break;

    case Moose::ElementNeighbor:
      J = -_test[_i][_qp] * (1.1 * _dsource_du_neigh[_qp] + 0.5 * _dsource_neigh_du_neigh[_qp]) *
          _phi_neighbor[_j][_qp];
      break;

    case Moose::NeighborElement:
      J = -_test_neighbor[_i][_qp] * (0.6 * _dsource_du[_qp] + 0.8 * _dsource_neigh_du[_qp]) *
          _phi[_j][_qp];
      break;

    case Moose::NeighborNeighbor:
      J = -_test_neighbor[_i][_qp] *
          (0.6 * _dsource_du_neigh[_qp] + 0.8 * _dsource_neigh_du_neigh[_qp]) *
          _phi_neighbor[_j][_qp];
      break;
  }

  return J;
}
