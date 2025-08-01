//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
GeochemistryApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;

  return params;
}

registerKnownLabel("GeochemistryApp");

GeochemistryApp::GeochemistryApp(const InputParameters & parameters) : MooseApp(parameters)
{
  GeochemistryApp::registerAll(_factory, _action_factory, _syntax);
}

GeochemistryApp::~GeochemistryApp() {}

void
GeochemistryApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  Registry::registerObjectsTo(f, {"GeochemistryApp"});
  Registry::registerActionsTo(af, {"GeochemistryApp"});

  registerSyntax("AddGeochemicalModelInterrogatorAction", "GeochemicalModelInterrogator");

  registerSyntax("AddTimeIndependentReactionSolverAction", "TimeIndependentReactionSolver");
  registerSyntax("AddTimeDependentReactionSolverAction", "TimeDependentReactionSolver");
  registerSyntax("AddSpatialReactionSolverAction", "SpatialReactionSolver");

  registerMooseObjectTask("add_geochemistry_reactor", AddGeochemistrySolverAction, false);
  addTaskDependency("add_geochemistry_reactor",
                    "add_user_object"); // depends on the GeochemicalModelDefinition

  registerMooseObjectTask("add_geochemistry_molality_aux", AddGeochemistrySolverAction, false);
  addTaskDependency("add_geochemistry_molality_aux",
                    "add_geochemistry_reactor"); // depends on the GeochemistryReactor
  addTaskDependency("add_distribution", "add_geochemistry_molality_aux");
}

void
GeochemistryApp::registerApps()
{
  registerApp(GeochemistryApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
GeochemistryApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  GeochemistryApp::registerAll(f, af, s);
}
extern "C" void
GeochemistryApp__registerApps()
{
  GeochemistryApp::registerApps();
}
