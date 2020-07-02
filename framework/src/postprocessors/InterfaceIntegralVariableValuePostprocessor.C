//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceIntegralVariableValuePostprocessor.h"
#include "InterfaceValueTools.h"

registerMooseObject("MooseApp", InterfaceIntegralVariableValuePostprocessor);

InputParameters
InterfaceIntegralVariableValuePostprocessor::validParams()
{
  InputParameters params = InterfaceIntegralPostprocessor::validParams();

  params.addRequiredCoupledVar("variable",
                               "The name of the variable on the primary side of the interface");
  params.addCoupledVar(
      "neighbor_variable",
      "The name of the variable on the secondary side of the interface. By default "
      "the same variable name is used for the secondary side");
  params.addClassDescription("Add access to variables and their gradient on an interface.");
  params.addParam<MooseEnum>("interface_value_type",
                             InterfaceValueTools::InterfaceAverageOptions(),
                             "Type of value we want to compute");
  return params;
}

InterfaceIntegralVariableValuePostprocessor::InterfaceIntegralVariableValuePostprocessor(
    const InputParameters & parameters)
  : InterfaceIntegralPostprocessor(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable")),
    _u_neighbor(parameters.isParamSetByUser("neighbor_variable")
                    ? coupledNeighborValue("neighbor_variable")
                    : coupledNeighborValue("variable")),
    _grad_u_neighbor(parameters.isParamSetByUser("neighbor_variable")
                         ? coupledNeighborGradient("neighbor_variable")
                         : coupledNeighborGradient("variable")),
    _interface_value_type(parameters.get<MooseEnum>("interface_value_type"))
{
  addMooseVariableDependency(mooseVariable());
}

Real
InterfaceIntegralVariableValuePostprocessor::computeQpIntegral()
{
  return InterfaceValueTools::getQuantity(_interface_value_type, _u[_qp], _u_neighbor[_qp]);
}
