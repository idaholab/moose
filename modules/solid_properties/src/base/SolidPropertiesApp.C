//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesApp.h"
#include "HeatConductionApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
SolidPropertiesApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("use_legacy_material_output") = false;
  return params;
}

registerKnownLabel("SolidPropertiesApp");

SolidPropertiesApp::SolidPropertiesApp(InputParameters parameters) : MooseApp(parameters)
{
  SolidPropertiesApp::registerAll(_factory, _action_factory, _syntax);
}

void
SolidPropertiesApp::registerApps()
{
  registerApp(SolidPropertiesApp);
}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerSyntaxTask(
      "AddSolidPropertiesDeprecatedAction", "Modules/SolidProperties/*", "add_solid_properties");
  registerSyntaxTask("AddSolidPropertiesAction", "SolidProperties/*", "add_solid_properties");
  registerMooseObjectTask("add_solid_properties", SolidProperties, false);

  syntax.addDependency("add_solid_properties", "add_function");
  syntax.addDependency("add_user_object", "add_solid_properties");
}

void
SolidPropertiesApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  HeatConductionApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"SolidPropertiesApp"});
  Registry::registerActionsTo(af, {"SolidPropertiesApp"});
  associateSyntaxInner(s, af);
}

void
SolidPropertiesApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"SolidPropertiesApp"});
}

void
SolidPropertiesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"SolidPropertiesApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
SolidPropertiesApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("use registerAll instead of registerExecFlags");
}

extern "C" void
SolidPropertiesApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SolidPropertiesApp::registerAll(f, af, s);
}
extern "C" void
SolidPropertiesApp__registerApps()
{
  SolidPropertiesApp::registerApps();
}
