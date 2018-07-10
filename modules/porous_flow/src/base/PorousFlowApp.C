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

  registerSyntaxTask("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_user_object");
  registerSyntaxTask("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_kernel");
  registerSyntaxTask("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_material");
  registerSyntaxTask("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_aux_variable");
  registerSyntaxTask("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_aux_kernel");

  registerSyntaxTask("PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_user_object");
  registerSyntaxTask("PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_kernel");
  registerSyntaxTask("PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_material");
  registerSyntaxTask("PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_aux_variable");
  registerSyntaxTask("PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_aux_kernel");

  registerSyntaxTask("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_user_object");
  registerSyntaxTask("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_kernel");
  registerSyntaxTask("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_material");
  registerSyntaxTask("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_aux_variable");
  registerSyntaxTask("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_aux_kernel");

  // Task dependency and syntax for action to automatically add PorousFlow materials
  registerSyntax("PorousFlowAddMaterialAction", "Materials");

  // Task dependency and syntax for action to automatically add PorousFlowJoiner materials
  registerTask("add_joiners", /*is_required=*/false);
  addTaskDependency("add_joiners", "add_material");

  registerSyntaxTask("PorousFlowAddMaterialJoiner", "Materials", "add_joiners");
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
