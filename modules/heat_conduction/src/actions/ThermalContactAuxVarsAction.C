//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalContactAuxVarsAction.h"
#include "FEProblem.h"
#include "AddVariableAction.h"

#include "libmesh/string_to_enum.h"

registerMooseAction("HeatConductionApp", ThermalContactAuxVarsAction, "add_aux_variable");

template <>
InputParameters
validParams<ThermalContactAuxVarsAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription(
      "Deprecated action that provide gap value name and gap conductivity name");
  params.addRequiredParam<NonlinearVariableName>("variable", "The variable for thermal contact");

  return params;
}

ThermalContactAuxVarsAction::ThermalContactAuxVarsAction(const InputParameters & params)
  : Action(params)
{
}
