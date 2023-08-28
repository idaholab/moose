//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ExecFlagRegistry.h"

#include "OptimizationAppTypes.h"

InputParameters
OptimizationApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy DirichletBC, that is, set DirichletBC default for preset = true
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

OptimizationApp::OptimizationApp(InputParameters parameters) : MooseApp(parameters)
{
  OptimizationApp::registerAll(_factory, _action_factory, _syntax);
}

OptimizationApp::~OptimizationApp() {}

void
OptimizationApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  Registry::registerObjectsTo(f, {"OptimizationApp"});
  Registry::registerActionsTo(af, {"OptimizationApp"});

  // Optimization reporter actions
  registerSyntaxTask(
      "AddOptimizationReporterAction", "OptimizationReporter", "add_optimization_reporter");
  registerMooseObjectTask("add_optimization_reporter", OptimizationReporterBase, false);
  addTaskDependency("add_optimization_reporter", "add_reporter");

  // General Optimization action
  registerTask("auto_create_mesh", false);
  registerTask("auto_create_problem", false);
  registerSyntaxTask("OptimizationAction", "Optimization", "auto_create_mesh");
  registerSyntaxTask("OptimizationAction", "Optimization", "auto_create_problem");
  addTaskDependency("setup_mesh", "auto_create_mesh");
  addTaskDependency("create_problem", "auto_create_problem");
}

void
OptimizationApp::registerApps()
{
  registerApp(OptimizationApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
OptimizationApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  OptimizationApp::registerAll(f, af, s);
}
extern "C" void
OptimizationApp__registerApps()
{
  OptimizationApp::registerApps();
}
