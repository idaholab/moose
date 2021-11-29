//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DebugResidualAux.h"
#include "NonlinearSystem.h"

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
