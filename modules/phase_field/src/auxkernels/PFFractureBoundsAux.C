//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PFFractureBoundsAux.h"
#include "SystemBase.h"

#include "MooseVariable.h"
#include "NonlinearSystem.h"

registerMooseObject("MooseApp", PFFractureBoundsAux);

template <>
InputParameters
validParams<PFFractureBoundsAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<Real>("upper", "The upper bound for the variable");
  params.addParam<Real>("lower", "The lower bound for the variable");
  params.addRequiredParam<NonlinearVariableName>(
      "lower_var",
      "Variable that acts as a lower bound to bounded_var.  It will not be "
      "constrained during the solution procedure");
  params.addRequiredCoupledVar("bounded_variable", "The variable to be bounded");
  return params;
}

PFFractureBoundsAux::PFFractureBoundsAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _upper_vector(_nl_sys.getVector("upper_bound")),
    _lower_vector(_nl_sys.getVector("lower_bound")),
    _bounded_variable_id(coupled("bounded_variable")),
    _lower_var_name(parameters.get<NonlinearVariableName>("lower_var")),
    _lower_var_num(0)
{
  if (!isNodal())
    mooseError("PFFractureBoundsAux must be used on a nodal auxiliary variable!");
  _upper_valid = parameters.isParamValid("upper");
  if (_upper_valid)
    _upper = getParam<Real>("upper");
  _lower_valid = parameters.isParamValid("lower");
  if (_lower_valid)
    _lower = getParam<Real>("lower");
}

Real
PFFractureBoundsAux::computeValue()
{
  if (_current_node->n_dofs(_nl_sys.number(), _bounded_variable_id) > 0)
  {
    // The zero is for the component, this will only work for Lagrange variables!
    dof_id_type dof = _current_node->dof_number(_nl_sys.number(), _bounded_variable_id, 0);

    MooseVariableFEBase & lower = _subproblem.getVariable(
        0, _lower_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);

    Real value_old = lower.getNodalValueOld(*_current_node);

    // if (_upper_valid)
    _upper_vector.set(dof, _upper);
    // if (_lower_valid)
    if (value_old < _lower)
      _lower_vector.set(dof, _lower);
    else
      _lower_vector.set(dof, value_old);
  }

  return 0.0;
}
