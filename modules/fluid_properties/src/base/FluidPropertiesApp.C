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
  Moose::registerObjects(_factory);
  FluidPropertiesApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  FluidPropertiesApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  FluidPropertiesApp::registerExecFlags(_factory);
}

FluidPropertiesApp::~FluidPropertiesApp() {}

// External entry point for dynamic application loading
extern "C" void
FluidPropertiesApp__registerApps()
{
  FluidPropertiesApp::registerApps();
}

void
FluidPropertiesApp::registerApps()
{
  registerApp(FluidPropertiesApp);
}

// External entry point for dynamic object registration
extern "C" void
FluidPropertiesApp__registerObjects(Factory & factory)
{
  FluidPropertiesApp::registerObjects(factory);
}

void
FluidPropertiesApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"FluidPropertiesApp"});
}

// External entry point for dynamic syntax association
extern "C" void
FluidPropertiesApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  FluidPropertiesApp::associateSyntax(syntax, action_factory);
}

void
FluidPropertiesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"FluidPropertiesApp"});

  registerSyntaxTask(
      "AddFluidPropertiesAction", "Modules/FluidProperties/*", "add_fluid_properties");

  registerMooseObjectTask("add_fluid_properties", FluidProperties, false);

  syntax.addDependency("add_fluid_properties", "init_displaced_problem");
}

// External entry point for dynamic execute flag registration
extern "C" void
FluidPropertiesApp__registerExecFlags(Factory & factory)
{
  FluidPropertiesApp::registerExecFlags(factory);
}
void
FluidPropertiesApp::registerExecFlags(Factory & /*factory*/)
{
}
