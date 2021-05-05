//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StructureAcousticInterface.h"
#include "MooseVariableFE.h"

registerMooseObject("FsiApp", StructureAcousticInterface);

InputParameters
StructureAcousticInterface::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addClassDescription("Enforces displacement and stress/pressure continuity "
                             "between the fluid and structural domains. Element "
                             "is always the structure and neighbor is always the"
                             " fluid.");
  params.addParam<MaterialPropertyName>("D", "D", "Fluid density.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component", "component < 3", "The desired component of displacement.");
  return params;
}

StructureAcousticInterface::StructureAcousticInterface(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _D(getMaterialProperty<Real>("D")),
    _neighbor_dotdot(_neighbor_var.uDotDotNeighbor()),
    _neighbor_dotdot_du(_neighbor_var.duDotDotDuNeighbor()),
    _component(getParam<unsigned int>("component"))
{
}

Real
StructureAcousticInterface::computeQpResidual(Moose::DGResidualType type)
{
  switch (type)
  {
    case Moose::Element: // Element is fluid
      return _test[_i][_qp] * _D[_qp] * _neighbor_dotdot[_qp] * _normals[_qp](_component);

    case Moose::Neighbor: // Neighbor is structure
      return _test_neighbor[_i][_qp] * -_u[_qp] * _normals[_qp](_component);
  }
  return 0.0;
}

Real
StructureAcousticInterface::computeQpJacobian(Moose::DGJacobianType type)
{
  switch (type)
  {
    case Moose::ElementElement:
      break;
    case Moose::NeighborNeighbor:
      break;
    case Moose::ElementNeighbor:
      return _test[_i][_qp] * _D[_qp] * _phi_neighbor[_j][_qp] * _neighbor_dotdot_du[_qp] *
             _normals[_qp](_component);
    case Moose::NeighborElement:
      return _test_neighbor[_i][_qp] * -_phi[_j][_qp] * _normals[_qp](_component);
  }
  return 0.0;
}
