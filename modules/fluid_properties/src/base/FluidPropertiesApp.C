//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidPropertiesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<FluidPropertiesApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("FluidPropertiesApp");

FluidPropertiesApp::FluidPropertiesApp(InputParameters parameters) : MooseApp(parameters)
{
  FluidPropertiesApp::registerAll(_factory, _action_factory, _syntax);
}

FluidPropertiesApp::~FluidPropertiesApp() {}

void
FluidPropertiesApp::registerApps()
{
  registerApp(FluidPropertiesApp);
}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerSyntaxTask(
      "AddFluidPropertiesAction", "Modules/FluidProperties/*", "add_fluid_properties");
  registerMooseObjectTask("add_fluid_properties", FluidProperties, false);
  syntax.addDependency("add_fluid_properties", "init_displaced_problem");
}

void
FluidPropertiesApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"FluidPropertiesApp"});
  Registry::registerActionsTo(af, {"FluidPropertiesApp"});
  associateSyntaxInner(s, af);
}

void
FluidPropertiesApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"FluidPropertiesApp"});
}

void
FluidPropertiesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"FluidPropertiesApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
FluidPropertiesApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("use registerAll instead of registerExecFlags");
}

extern "C" void
FluidPropertiesApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FluidPropertiesApp::registerAll(f, af, s);
}
extern "C" void
FluidPropertiesApp__registerApps()
{
  FluidPropertiesApp::registerApps();
}
