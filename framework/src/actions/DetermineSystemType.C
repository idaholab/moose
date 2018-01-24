//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DetermineSystemType.h"
#include "MooseApp.h"

template <>
InputParameters
validParams<DetermineSystemType>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.mooseObjectSyntaxVisibility(false);
  return params;
}

DetermineSystemType::DetermineSystemType(InputParameters parameters) : MooseObjectAction(parameters)
{
}

void
DetermineSystemType::act()
{
  /**
   * Determine whether the Executioner is derived from EigenExecutionerBase and
   * set a flag on MooseApp that can be used during problem construction.
   */
  if (_moose_object_pars.isParamValid("_eigen") && _moose_object_pars.get<bool>("_eigen"))
    _app.useNonlinear() = false;

  auto _action_params = _action_factory.getValidParams("CreateProblemAction");

  if (_moose_object_pars.isParamValid("_use_eigen_value") &&
      _moose_object_pars.get<bool>("_use_eigen_value"))
  {
    _app.useEigenvalue() = true;
    _action_params.set<std::string>("type") = "EigenProblem";
  }
  else
  {
    _action_params.set<std::string>("type") = "FEProblem";
  }

  // if we have Problem block in input file, "CreateProblemAction" will be handled by parser
  if (_awh.hasActions("create_problem"))
    return;

  // Create "CreateProblemAction"
  auto action = _action_factory.create(
      "CreateProblemAction", _action_params.get<std::string>("type"), _action_params);

  // Add the action to the warehouse
  _awh.addActionBlock(action);
}
