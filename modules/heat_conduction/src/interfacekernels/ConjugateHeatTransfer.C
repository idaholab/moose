//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConjugateHeatTransfer.h"

#include "metaphysicl/raw_type.h"

using MetaPhysicL::raw_value;

registerMooseObject("HeatConductionApp", ConjugateHeatTransfer);

InputParameters
ConjugateHeatTransfer::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("htc", "heat transfer coefficient");
  params.addRequiredCoupledVar("T_fluid",
                               "The fluid temperature. It is not always identical to neighbor_var, "
                               "e.g. when the fluid heat equation is solved for internal energy");
  params.addClassDescription(
      "This InterfaceKernel models conjugate heat transfer. Fluid side must "
      "be primary side and solid side must be secondary side. T_fluid is provided in case that "
      "variable "
      "(== fluid energy variable) is not temperature but e.g. internal energy.");
  return params;
}

ConjugateHeatTransfer::ConjugateHeatTransfer(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _h(getADMaterialProperty<Real>("htc")),
    _T_fluid(coupledValue("T_fluid")),
    _apply_element_jacobian(_var.name() == getParam<std::vector<VariableName>>("T_fluid")[0])
{
}

Real
ConjugateHeatTransfer::computeQpResidual(Moose::DGResidualType type)
{
  switch (type)
  {
    case Moose::Element:
      return raw_value(_h[_qp]) * (_T_fluid[_qp] - _neighbor_value[_qp]) * _test[_i][_qp];

    case Moose::Neighbor:
      return raw_value(_h[_qp]) * (_neighbor_value[_qp] - _T_fluid[_qp]) * _test_neighbor[_i][_qp];

    default:
      return 0.0;
  }
}

Real
ConjugateHeatTransfer::computeQpJacobian(Moose::DGJacobianType type)
{
  switch (type)
  {
    case Moose::ElementElement:
      return _apply_element_jacobian ? raw_value(_h[_qp]) * _phi[_j][_qp] * _test[_i][_qp] : 0;

    case Moose::NeighborNeighbor:
      return raw_value(_h[_qp]) * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];

    case Moose::NeighborElement:
      return _apply_element_jacobian ? raw_value(_h[_qp]) * -_phi[_j][_qp] * _test_neighbor[_i][_qp]
                                     : 0;

    case Moose::ElementNeighbor:
      return raw_value(_h[_qp]) * -_phi_neighbor[_j][_qp] * _test[_i][_qp];

    default:
      return 0.0;
  }
}
