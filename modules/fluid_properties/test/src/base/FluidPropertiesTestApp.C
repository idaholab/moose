//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "FluidPropertiesTestApp.h"
#include "FluidPropertiesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "MultiComponentFluidPropertiesMaterialPT.h"

template <>
InputParameters
validParams<FluidPropertiesTestApp>()
{
  InputParameters params = validParams<FluidPropertiesApp>();
  return params;
}

FluidPropertiesTestApp::FluidPropertiesTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  FluidPropertiesApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  FluidPropertiesApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  FluidPropertiesApp::registerExecFlags(_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    FluidPropertiesTestApp::registerObjects(_factory);
    FluidPropertiesTestApp::associateSyntax(_syntax, _action_factory);
  }
}

FluidPropertiesTestApp::~FluidPropertiesTestApp() {}

// External entry point for dynamic application loading
extern "C" void
FluidPropertiesTestApp__registerApps()
{
  FluidPropertiesTestApp::registerApps();
}
void
FluidPropertiesTestApp::registerApps()
{
  registerApp(FluidPropertiesApp);
  registerApp(FluidPropertiesTestApp);
}

// External entry point for dynamic object registration
extern "C" void
FluidPropertiesTestApp__registerObjects(Factory & factory)
{
  FluidPropertiesTestApp::registerObjects(factory);
}
void
FluidPropertiesTestApp::registerObjects(Factory & factory)
{
  registerMaterial(MultiComponentFluidPropertiesMaterialPT);
}

// External entry point for dynamic syntax association
extern "C" void
FluidPropertiesTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  FluidPropertiesTestApp::associateSyntax(syntax, action_factory);
}
void
FluidPropertiesTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

// External entry point for dynamic execute flag registration
extern "C" void
FluidPropertiesTestApp__registerExecFlags(Factory & factory)
{
  FluidPropertiesTestApp::registerExecFlags(factory);
}
void
FluidPropertiesTestApp::registerExecFlags(Factory & /*factory*/)
{
}
