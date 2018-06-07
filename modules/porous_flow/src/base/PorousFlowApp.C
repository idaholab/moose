//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "PorousFlowApp.h"
#include "Moose.h"
#include "TensorMechanicsApp.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "FluidPropertiesApp.h"
#include "ChemicalReactionsApp.h"

template <>
InputParameters
validParams<PorousFlowApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("PorousFlowApp");

PorousFlowApp::PorousFlowApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  PorousFlowApp::registerObjectDepends(_factory);
  PorousFlowApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  PorousFlowApp::associateSyntaxDepends(_syntax, _action_factory);
  PorousFlowApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  PorousFlowApp::registerExecFlags(_factory);
}

PorousFlowApp::~PorousFlowApp() {}

// External entry point for dynamic application loading
extern "C" void
PorousFlowApp__registerApps()
{
  PorousFlowApp::registerApps();
}
void
PorousFlowApp::registerApps()
{
  registerApp(PorousFlowApp);
}

void
PorousFlowApp::registerObjectDepends(Factory & factory)
{
  TensorMechanicsApp::registerObjects(factory);
  FluidPropertiesApp::registerObjects(factory);
  ChemicalReactionsApp::registerObjects(factory);
}

// External entry point for dynamic object registration
extern "C" void
PorousFlowApp__registerObjects(Factory & factory)
{
  PorousFlowApp::registerObjects(factory);
}
void
PorousFlowApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"PorousFlowApp"});
}

void
PorousFlowApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
  FluidPropertiesApp::associateSyntax(syntax, action_factory);
  ChemicalReactionsApp::associateSyntax(syntax, action_factory);
}

// External entry point for dynamic syntax association
extern "C" void
PorousFlowApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  PorousFlowApp::associateSyntax(syntax, action_factory);
}
void
PorousFlowApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"PorousFlowApp"});

  syntax.registerActionSyntax("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_user_object");
  syntax.registerActionSyntax("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_kernel");
  syntax.registerActionSyntax("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_material");
  syntax.registerActionSyntax("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_aux_variable");
  syntax.registerActionSyntax("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_aux_kernel");

  syntax.registerActionSyntax(
      "PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_user_object");
  syntax.registerActionSyntax("PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_kernel");
  syntax.registerActionSyntax(
      "PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_material");
  syntax.registerActionSyntax(
      "PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_aux_variable");
  syntax.registerActionSyntax(
      "PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_aux_kernel");

  syntax.registerActionSyntax("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_user_object");
  syntax.registerActionSyntax("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_kernel");
  syntax.registerActionSyntax("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_material");
  syntax.registerActionSyntax("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_aux_variable");
  syntax.registerActionSyntax("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_aux_kernel");

  registerTask("add_joiners", /*is_required=*/false);
  addTaskDependency("add_joiners", "add_material");
  addTaskDependency("add_joiners", "add_user_object");

  syntax.registerActionSyntax("PorousFlowAddMaterialJoiner", "Materials", "add_joiners");
}

// External entry point for dynamic execute flag registration
extern "C" void
PorousFlowApp__registerExecFlags(Factory & factory)
{
  PorousFlowApp::registerExecFlags(factory);
}
void
PorousFlowApp::registerExecFlags(Factory & /*factory*/)
{
}
