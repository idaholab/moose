//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

#include "FEProblemBase.h"

CoupleableMooseVariableDependencyIntermediateInterface::
    CoupleableMooseVariableDependencyIntermediateInterface(const MooseObject * moose_object,
                                                           bool nodal,
                                                           bool is_fv)
  : Coupleable(moose_object, nodal, is_fv),
    ScalarCoupleable(moose_object),
    MooseVariableDependencyInterface(moose_object)
{
  for (MooseVariableFEBase * coupled_var : getCoupledMooseVars())
    addMooseVariableDependency(coupled_var);
}

const VariableValue &
CoupleableMooseVariableDependencyIntermediateInterface::coupledValueByName(
    const std::string & var_name)
{
  MooseVariableFieldBase * moose_var = &_c_fe_problem.getVariable(
      _c_tid, var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
  _coupled_moose_vars.push_back(moose_var);
  MooseVariable * var = &_c_fe_problem.getStandardVariable(_c_tid, var_name);
  _coupled_standard_moose_vars.push_back(var);

  addMooseVariableDependency(moose_var);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValues() : var->dofValuesOld();
    else
      return (_c_is_implicit) ? var->sln() : var->slnOld();
  }
  else
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
    else
      return (_c_is_implicit) ? var->slnNeighbor() : var->slnOldNeighbor();
  }
}

const ArrayVariableValue &
CoupleableMooseVariableDependencyIntermediateInterface::coupledArrayValueByName(
    const std::string & var_name)
{
  MooseVariableFieldBase * moose_var = &_c_fe_problem.getVariable(
      _c_tid, var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
  _coupled_moose_vars.push_back(moose_var);
  ArrayMooseVariable * var = &_c_fe_problem.getArrayVariable(_c_tid, var_name);
  _coupled_array_moose_vars.push_back(var);

  addMooseVariableDependency(moose_var);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValues() : var->dofValuesOld();
    else
      return (_c_is_implicit) ? var->sln() : var->slnOld();
  }
  else
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
    else
      return (_c_is_implicit) ? var->slnNeighbor() : var->slnOldNeighbor();
  }
}
