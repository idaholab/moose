//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetupDebugAction.h"
#include "FEProblem.h"
#include "ActionWarehouse.h"
#include "Factory.h"
#include "Output.h"
#include "MooseApp.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"

registerMooseAction("MooseApp", SetupDebugAction, "add_output");

template <>
InputParameters
validParams<SetupDebugAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<unsigned int>(
      "show_top_residuals", 0, "The number of top residuals to print out (0 = no output)");
  params.addParam<bool>(
      "show_var_residual_norms",
      false,
      "Print the residual norms of the individual solution variables at each nonlinear iteration");
  params.addParam<bool>("show_actions", false, "Print out the actions being executed");
  params.addParam<bool>(
      "show_parser", false, "Shows parser block extraction and debugging information");
  params.addParam<bool>(
      "show_material_props",
      false,
      "Print out the material properties supplied for each block, face, neighbor, and/or sideset");

  params.addClassDescription(
      "Adds various debugging type Outputters to the simulation system based on user parameters");

  return params;
}

SetupDebugAction::SetupDebugAction(InputParameters parameters) : Action(parameters)
{
  _awh.showActions(getParam<bool>("show_actions"));
  _awh.showParser(getParam<bool>("show_parser"));
}

void
SetupDebugAction::act()
{
  // Material properties
  if (_pars.get<bool>("show_material_props"))
  {
    const std::string type = "MaterialPropertyDebugOutput";
    auto params = _factory.getValidParams(type);
    _problem->addOutput(type, "_moose_material_property_debug_output", params);
  }

  // Variable residusl norms
  if (_pars.get<bool>("show_var_residual_norms"))
  {
    const std::string type = "VariableResidualNormsDebugOutput";
    auto params = _factory.getValidParams(type);
    _problem->addOutput(type, "_moose_variable_residual_norms_debug_output", params);
  }

  // Top residuals
  if (_pars.get<unsigned int>("show_top_residuals") > 0)
  {
    const std::string type = "TopResidualDebugOutput";
    auto params = _factory.getValidParams(type);
    params.set<unsigned int>("num_residuals") = _pars.get<unsigned int>("show_top_residuals");
    _problem->addOutput(type, "_moose_top_residual_debug_output", params);
  }
}
