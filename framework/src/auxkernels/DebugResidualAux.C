//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DebugResidualAux.h"
#include "NonlinearSystem.h"

#include "libmesh/string_to_enum.h"

registerMooseObject("MooseApp", DebugResidualAux);

InputParameters
DebugResidualAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Populate an auxiliary variable with the residual contribution of a variable.");
  params.addRequiredParam<NonlinearVariableName>("debug_variable",
                                                 "The variable that is being debugged.");
  return params;
}

DebugResidualAux::DebugResidualAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _debug_var(_nl_sys.getVariable(_tid, getParam<NonlinearVariableName>("debug_variable"))),
    _residual_copy(_nl_sys.residualGhosted())
{
  // Check that variable order/family match aux_variable order/family
  auto var_order = Utility::string_to_enum<Order>(_var.getParam<MooseEnum>("order"));
  auto debug_order = Utility::string_to_enum<Order>(_debug_var.getParam<MooseEnum>("order"));
  auto var_family = Utility::string_to_enum<FEFamily>(_var.getParam<MooseEnum>("family"));
  auto debug_family = Utility::string_to_enum<FEFamily>(_debug_var.getParam<MooseEnum>("family"));
  if (var_order != debug_order || var_family != debug_family)
    paramError("variable",
               "A mismatch was found between family and order parameters for ",
               _var.name(),
               " and ",
               _debug_var.name());
  if (!_nodal && debug_order > 0)
    mooseWarning("Residual output is approximate for variable order " +
                 Moose::stringify(debug_order));
}

Real
DebugResidualAux::computeValue()
{
  if (_nodal)
  {
    dof_id_type dof = _current_node->dof_number(_nl_sys.number(), _debug_var.number(), 0);
    return _residual_copy(dof);
  }
  else
  {
    dof_id_type dof = _current_elem->dof_number(_nl_sys.number(), _debug_var.number(), 0);
    return _residual_copy(dof);
  }
}
