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
#include "AddAuxVariableAction.h"
#include "MooseUtils.h"

registerMooseAction("MooseApp", SetupDebugAction, "add_output");

InputParameters
SetupDebugAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<unsigned int>(
      "show_top_residuals", 0, "The number of top residuals to print out (0 = no output)");
  params.addParam<bool>(
      "show_var_residual_norms",
      false,
      "Print the residual norms of the individual solution variables at each nonlinear iteration");
  params.addParam<bool>("show_action_dependencies", false, "Print out the action dependencies");
  params.addParam<bool>("show_actions", false, "Print out the actions being executed");
  params.addParam<bool>(
      "show_parser", false, "Shows parser block extraction and debugging information");
  params.addParam<bool>(
      "show_material_props",
      false,
      "Print out the material properties supplied for each block, face, neighbor, and/or sideset");
  params.addParam<bool>("show_mesh_meta_data", false, "Print out the available mesh meta data");
  params.addParam<bool>(
      "show_reporters", false, "Print out information about the declared and requested Reporters");

  ExecFlagEnum print_on = MooseUtils::getDefaultExecFlagEnum();
  print_on.addAvailableFlags(EXEC_TRANSFER);
  print_on.addAvailableFlags(EXEC_FAILED);
  params.addParam<ExecFlagEnum>(
      "show_execution_order",
      print_on,
      "Print more information about the order of execution during calculations");
  params.addDeprecatedParam<bool>(
      "pid_aux",
      "Add a AuxVariable named \"pid\" that shows the processors and partitioning",
      "pid_aux is deprecated, use output_process_domains");
  params.addParam<bool>(
      "output_process_domains",
      false,
      "Add a AuxVariable named \"pid\" that shows the partitioning for each process");
  params.addParam<bool>(
      "show_functors", false, "Whether to print information about the functors in the problem");

  params.addClassDescription("Adds various debugging type output to the simulation system.");

  return params;
}

SetupDebugAction::SetupDebugAction(const InputParameters & parameters) : Action(parameters)
{
  _awh.showActionDependencies(getParam<bool>("show_action_dependencies"));
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

  // Variable residual norms
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

  // Print full names of mesh meta data
  if (getParam<bool>("show_mesh_meta_data"))
  {
    _console << "Mesh meta data:\n";
    for (auto it = _app.getRestartableDataMapBegin(); it != _app.getRestartableDataMapEnd(); ++it)
      if (it->first == MooseApp::MESH_META_DATA)
        for (auto & pair : it->second.first)
          _console << " " << pair.first << std::endl;
  }

  // Print Reporter information
  if (getParam<bool>("show_reporters"))
  {
    const std::string type = "ReporterDebugOutput";
    auto params = _factory.getValidParams(type);
    _problem->addOutput(type, "_moose_reporter_debug_output", params);
  }

  // Print execution information in all loops
  if (parameters().isParamSetByUser("show_execution_order"))
    _problem->setExecutionPrinting(getParam<ExecFlagEnum>("show_execution_order"));

  // Add pid aux
  if (getParam<bool>("output_process_domains") ||
      (isParamValid("pid_aux") && getParam<bool>("pid_aux")))
  {
    if (_problem->hasVariable("pid"))
      paramError("output_process_domains", "Variable with the name \"pid\" already exists");

    auto fe_type = FEType(CONSTANT, MONOMIAL);
    auto type = AddAuxVariableAction::variableType(fe_type);
    auto var_params = _factory.getValidParams(type);
    _problem->addAuxVariable(type, "pid", var_params);

    InputParameters params = _factory.getValidParams("ProcessorIDAux");
    params.set<AuxVariableName>("variable") = "pid";
    _problem->addAuxKernel("ProcessorIDAux", "pid_aux", params);
  }

  // Add functor output
  if (getParam<bool>("show_functors"))
    _problem->setFunctorOutput(getParam<bool>("show_functors"));
}
