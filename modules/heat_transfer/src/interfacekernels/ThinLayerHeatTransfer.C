//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThinLayerHeatTransfer.h"

registerMooseObject("HeatConductionApp", ThinLayerHeatTransfer);

InputParameters
ThinLayerHeatTransfer::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addClassDescription("Model heat transfer across a thin domain with an interface.");
  params.addParam<MaterialPropertyName>(
      "specific_heat", "Property name of the specific heat material property of the thin layer");

  params.addParam<MaterialPropertyName>(
      "density", "Property name of the density material property of the thin layer");

  params.addParam<MaterialPropertyName>(
      "heat_source", "Property name of the heat source material property of the thin layer");

  params.addParam<MaterialPropertyName>(
      "thermal_conductivity", "thermal_conductivity", "Property name of the thermal conductivity");

  params.addRequiredParam<Real>("thickness", "The thin layer thickness");
  return params;
}

ThinLayerHeatTransfer::ThinLayerHeatTransfer(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _specific_heat(parameters.isParamValid("specific_heat")
                       ? getMaterialProperty<Real>("specific_heat")
                       : getGenericZeroMaterialProperty<Real, false>()),
    _density(parameters.isParamValid("density") ? getMaterialProperty<Real>("density")
                                                : getGenericZeroMaterialProperty<Real, false>()),
    _heat_source(parameters.isParamValid("heat_source")
                     ? getMaterialProperty<Real>("heat_source")
                     : getGenericZeroMaterialProperty<Real, false>()),
    _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),
    _thickness(getParam<Real>("thickness")),
    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu()),
    _u_dot_neighbor(_var.uDotNeighbor()),
    _du_dot_du_neighbor(_var.duDotDuNeighbor())
{
}

Real
ThinLayerHeatTransfer::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;

  switch (type)
  {
    case Moose::Element:
      r = -_test[_i][_qp] * (0.5 * _thickness * _heat_source[_qp] -
                             _density[_qp] * _specific_heat[_qp] * 0.5 * _thickness * _u_dot[_qp] -
                             (-_thermal_conductivity[_qp] * (_neighbor_value[_qp] - _u[_qp]) /
                              std::max(_thickness, libMesh::TOLERANCE)));
      break;
    case Moose::Neighbor:
      r = -_test_neighbor[_i][_qp] *
          (0.5 * _thickness * _heat_source[_qp] -
           _density[_qp] * _specific_heat[_qp] * 0.5 * _thickness * _u_dot_neighbor[_qp] -
           (-_thermal_conductivity[_qp] * (_u[_qp] - _neighbor_value[_qp]) /
            (std::max(_thickness, libMesh::TOLERANCE))));
      break;
  }
  return r;
}

Real
ThinLayerHeatTransfer::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0;

  switch (type)
  {
    case Moose::ElementElement:
      jac = -_test[_i][_qp] * (-_density[_qp] * _specific_heat[_qp] * 0.5 * _thickness *
                                   _du_dot_du[_qp] * _phi[_j][_qp] -
                               (-_thermal_conductivity[_qp] * -_phi[_j][_qp] /
                                std::max(_thickness, libMesh::TOLERANCE)));
      break;
    case Moose::NeighborNeighbor:
      jac = -_test_neighbor[_i][_qp] * (-_density[_qp] * _specific_heat[_qp] * 0.5 * _thickness *
                                            _du_dot_du_neighbor[_qp] * _phi_neighbor[_j][_qp] -
                                        (-_thermal_conductivity[_qp] * -_phi_neighbor[_j][_qp] /
                                         std::max(_thickness, libMesh::TOLERANCE)));
      break;

    case Moose::NeighborElement:
      jac = -_test_neighbor[_i][_qp] * (-(-_thermal_conductivity[_qp] * _phi[_j][_qp] /
                                          std::max(_thickness, libMesh::TOLERANCE)));
      break;

    case Moose::ElementNeighbor:
      jac = -_test[_i][_qp] * (-(-_thermal_conductivity[_qp] * _phi_neighbor[_j][_qp] /
                                 std::max(_thickness, libMesh::TOLERANCE)));
      break;
  }

  return jac;
}
