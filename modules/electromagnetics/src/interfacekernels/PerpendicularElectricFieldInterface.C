//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerpendicularElectricFieldInterface.h"

registerMooseObject("ElectromagneticsApp", PerpendicularElectricFieldInterface);

InputParameters
PerpendicularElectricFieldInterface::validParams()
{
  InputParameters params = VectorInterfaceKernel::validParams();
  params.addClassDescription(
      "VectorInterfaceKernel that implements the condition $\\epsilon_1 \\vec{E}_{1}^{\\perp} - "
      "\\epsilon_2 \\vec{E}_{2}^{\\perp} = \\sigma_f$");
  params.addParam<Real>("free_charge", 0.0, "Free charge on the interface (default = 0).");
  params.addParam<Real>(
      "primary_epsilon", 1.0, "Permittivity on the primary side of the interface (default = 1.0).");
  params.addParam<Real>("secondary_epsilon",
                        1.0,
                        "Permittivity on the secondary side of the interface (default = 1.0).");
  return params;
}

PerpendicularElectricFieldInterface::PerpendicularElectricFieldInterface(
    const InputParameters & parameters)
  : VectorInterfaceKernel(parameters),

    _free_charge(getParam<Real>("free_charge")),
    _primary_eps(getParam<Real>("primary_epsilon")),
    _secondary_eps(getParam<Real>("secondary_epsilon"))
{
}

Real
PerpendicularElectricFieldInterface::computeQpResidual(Moose::DGResidualType type)
{
  _u_perp = (_u[_qp] * _normals[_qp]) * _normals[_qp];
  _secondary_perp = (_neighbor_value[_qp] * _normals[_qp]) * _normals[_qp];
  _free_charge_dot_n = _free_charge * _normals[_qp];

  Real res = 0;

  switch (type)
  {
    case Moose::Element:
      res = _test[_i][_qp] *
            (_primary_eps * _u_perp - _secondary_eps * _secondary_perp - _free_charge_dot_n);
      break;

    case Moose::Neighbor:
      res = _test_neighbor[_i][_qp] *
            -(_primary_eps * _u_perp - _secondary_eps * _secondary_perp - _free_charge_dot_n);
      break;
  }

  return res;
}

Real
PerpendicularElectricFieldInterface::computeQpJacobian(Moose::DGJacobianType type)
{
  _phi_u_perp = (_phi[_j][_qp] * _normals[_qp]) * _normals[_qp];
  _phi_secondary_perp = (_phi_neighbor[_j][_qp] * _normals[_qp]) * _normals[_qp];

  Real jac = 0;

  switch (type)
  {
    case Moose::ElementElement:
      jac = _test[_i][_qp] * _primary_eps * _phi_u_perp;
      break;

    case Moose::NeighborNeighbor:
      jac = _test_neighbor[_i][_qp] * _secondary_eps * _phi_secondary_perp;
      break;

    case Moose::NeighborElement:
      jac = _test_neighbor[_i][_qp] * -_primary_eps * _phi_u_perp;
      break;

    case Moose::ElementNeighbor:
      jac = _test[_i][_qp] * -_secondary_eps * _phi_secondary_perp;
      break;
  }

  return jac;
}
