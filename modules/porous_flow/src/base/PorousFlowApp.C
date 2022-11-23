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

InputParameters
PorousFlowApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

registerKnownLabel("PorousFlowApp");

PorousFlowApp::PorousFlowApp(const InputParameters & parameters) : MooseApp(parameters)
{
  PorousFlowApp::registerAll(_factory, _action_factory, _syntax);
}

PorousFlowApp::~PorousFlowApp() {}

void
PorousFlowApp::registerApps()
{
  registerApp(PorousFlowApp);
}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
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

  syntax.registerActionSyntax("PorousFlowAddBCAction", "Modules/PorousFlow/BCs/*");

  registerMooseObjectTask("add_porous_flow_bc", PorousFlowSinkBC, false);
  addTaskDependency("add_porous_flow_bc", "add_bc");
  addTaskDependency("resolve_optional_materials", "add_porous_flow_bc");

  // Task dependency and syntax for action to automatically add PorousFlow materials
  registerSyntax("PorousFlowAddMaterialAction", "Materials");

  // Task dependency and syntax for action to automatically add PorousFlowJoiner materials
  registerTask("add_joiners", /*is_required=*/false);
  addTaskDependency("add_joiners", "add_material");
  addTaskDependency("add_master_action_material", "add_joiners");

  registerSyntaxTask("PorousFlowAddMaterialJoiner", "Materials", "add_joiners");
}

void
PorousFlowApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  TensorMechanicsApp::registerAll(f, af, s);
  FluidPropertiesApp::registerAll(f, af, s);
  ChemicalReactionsApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"PorousFlowApp"});
  Registry::registerActionsTo(af, {"PorousFlowApp"});
  associateSyntaxInner(s, af);
}

void
PorousFlowApp::registerObjectDepends(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjectsDepends");
  TensorMechanicsApp::registerObjects(factory);
  FluidPropertiesApp::registerObjects(factory);
  ChemicalReactionsApp::registerObjects(factory);
}

void
PorousFlowApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"PorousFlowApp"});
}

void
PorousFlowApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntaxDepends");
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
  FluidPropertiesApp::associateSyntax(syntax, action_factory);
  ChemicalReactionsApp::associateSyntax(syntax, action_factory);
}

void
PorousFlowApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"PorousFlowApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
PorousFlowApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

extern "C" void
PorousFlowApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  PorousFlowApp::registerAll(f, af, s);
}
extern "C" void
PorousFlowApp__registerApps()
{
  PorousFlowApp::registerApps();
}
