//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetHeatTransferKernel.h"

registerMooseObject("HeatConductionApp", SideSetHeatTransferKernel);

InputParameters
SideSetHeatTransferKernel::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addClassDescription(
      "Modeling conduction, convection, and radiation across internal side set.");
  params.addParam<MaterialPropertyName>("conductance",
                                        "gap_conductance",
                                        "Conductivity of gap divided by effective gap width,"
                                        "conductance ignored if not provided");
  params.addCoupledVar("Tbulk_var", "Bulk temperature of gap as variable");
  params.addParam<MaterialPropertyName>(
      "Tbulk_mat", "gap_Tbulk", "Bulk temperature of gap as material property");
  params.addParam<MaterialPropertyName>(
      "h_primary",
      "gap_h_primary",
      "Convective heat transfer coefficient (primary face), convection ignored if not provided");
  params.addParam<MaterialPropertyName>(
      "h_neighbor",
      "gap_h_neighbor",
      "Convective heat transfer coefficient (neighbor face), convection ignored if not provided");
  params.addParam<MaterialPropertyName>(
      "emissivity_eff_primary",
      "gap_emissivity_eff_primary",
      "Effective emmissivity of primary face, radiation ignored if not provided. "
      "This value contains contributions from reflectivity, see SideSetHeatTransferMaterial "
      "for details.");
  params.addParam<MaterialPropertyName>(
      "emissivity_eff_neighbor",
      "gap_emissivity_eff_neighbor",
      "Effective emmissivity of neighbor face, radiation ignored if not provided. "
      "This value contains contributions from reflectivity, see SideSetHeatTransferMaterial "
      "for details.");
  return params;
}

SideSetHeatTransferKernel::SideSetHeatTransferKernel(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _cond(getMaterialProperty<Real>("conductance")),
    _Tbulk_var(isParamValid("Tbulk_var") ? &coupledValue("Tbulk_var") : nullptr),
    _Tbulk_mat(_Tbulk_var ? nullptr : &getMaterialProperty<Real>("Tbulk_mat")),
    _hp(getMaterialProperty<Real>("h_primary")),
    _hm(getMaterialProperty<Real>("h_neighbor")),
    _eps_p(getMaterialProperty<Real>("emissivity_eff_primary")),
    _eps_m(getMaterialProperty<Real>("emissivity_eff_neighbor"))
{
  if (parameters.isParamSetByUser("Tbulk_mat") && _Tbulk_var)
    paramError("Tbulk_var", "Both Tbulk_mat and Tbulk_var set by user, cannot use both.");

  if (_var.number() == _neighbor_var.number() && _var.isNodal())
    mooseError(
        "Variable and neighbor variable are the same, but they are not elemental variables.");
}

Real
SideSetHeatTransferKernel::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;

  if (_cond[_qp] != 0.0) // Conduction
  {
    Real jump = _u[_qp] - _neighbor_value[_qp];
    switch (type)
    {
      case Moose::Element:
        r += _cond[_qp] * jump * _test[_i][_qp];
        break;

      case Moose::Neighbor:
        r -= _cond[_qp] * jump * _test_neighbor[_i][_qp];
        break;
    }
  }

  if (_hp[_qp] != 0.0 && _hm[_qp] != 0.0) // Convection
  {
    Real Tb = (_Tbulk_var ? (*_Tbulk_var)[_qp] : (*_Tbulk_mat)[_qp]);
    switch (type)
    {
      case Moose::Element:
        r += _hp[_qp] * (_u[_qp] - Tb) * _test[_i][_qp];
        break;

      case Moose::Neighbor:
        r += _hm[_qp] * (_neighbor_value[_qp] - Tb) * _test_neighbor[_i][_qp];
        break;
    }
  }

  if (_eps_p[_qp] != 0.0 && _eps_m[_qp] != 0.0) // Radiation
  {
    Real Rp = _eps_p[_qp] * (_u[_qp] * _u[_qp] * _u[_qp] * _u[_qp]);
    Real Rm = _eps_m[_qp] * (_neighbor_value[_qp] * _neighbor_value[_qp] * _neighbor_value[_qp] *
                             _neighbor_value[_qp]);
    switch (type)
    {
      case Moose::Element:
        r += (Rp - Rm) * _test[_i][_qp];
        break;

      case Moose::Neighbor:
        r += (Rm - Rp) * _test_neighbor[_i][_qp];
        break;
    }
  }

  return r;
}

Real
SideSetHeatTransferKernel::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0;

  if (_cond[_qp] != 0.0) // Conduction
  {
    switch (type)
    {
      case Moose::ElementElement:
        jac += _cond[_qp] * _phi[_j][_qp] * _test[_i][_qp];
        break;

      case Moose::NeighborNeighbor:
        jac += _cond[_qp] * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
        break;

      case Moose::ElementNeighbor:
        jac -= _cond[_qp] * _phi_neighbor[_j][_qp] * _test[_i][_qp];
        break;

      case Moose::NeighborElement:
        jac -= _cond[_qp] * _phi[_j][_qp] * _test_neighbor[_i][_qp];
        break;
    }
  }

  if (_hp[_qp] != 0.0 && _hm[_qp] != 0.0) // Convection
  {
    switch (type)
    {
      case Moose::ElementElement:
        jac += _hp[_qp] * _phi[_j][_qp] * _test[_i][_qp];
        break;

      case Moose::NeighborNeighbor:
        jac += _hm[_qp] * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
        break;

      case Moose::NeighborElement:
      case Moose::ElementNeighbor:
        break;
    }
  }

  if (_eps_p[_qp] != 0.0 && _eps_m[_qp] != 0.0) // Radiation
  {
    switch (type)
    {
      case Moose::ElementElement:
        jac += 4.0 * _eps_p[_qp] * (_u[_qp] * _u[_qp] * _u[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
        break;

      case Moose::NeighborNeighbor:
        jac += 4.0 * _eps_m[_qp] *
               (_neighbor_value[_qp] * _neighbor_value[_qp] * _neighbor_value[_qp]) *
               _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
        break;

      case Moose::ElementNeighbor:
        jac -= 4.0 * _eps_m[_qp] *
               (_neighbor_value[_qp] * _neighbor_value[_qp] * _neighbor_value[_qp]) *
               _phi_neighbor[_j][_qp] * _test[_i][_qp];
        break;

      case Moose::NeighborElement:
        jac -= 4.0 * _eps_p[_qp] * (_u[_qp] * _u[_qp] * _u[_qp]) * _phi[_j][_qp] *
               _test_neighbor[_i][_qp];
        break;
    }
  }

  return jac;
}
