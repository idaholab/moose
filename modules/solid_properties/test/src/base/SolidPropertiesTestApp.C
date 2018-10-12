//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "SolidPropertiesTestApp.h"
#include "SolidPropertiesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<SolidPropertiesTestApp>()
{
  InputParameters params = validParams<SolidPropertiesApp>();
  return params;
}

registerKnownLabel("SolidPropertiesTestApp");

SolidPropertiesTestApp::SolidPropertiesTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  SolidPropertiesApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  SolidPropertiesApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  SolidPropertiesApp::registerExecFlags(_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    SolidPropertiesTestApp::registerObjects(_factory);
    SolidPropertiesTestApp::associateSyntax(_syntax, _action_factory);
  }
}

SolidPropertiesTestApp::~SolidPropertiesTestApp() {}

// External entry point for dynamic application loading
extern "C" void
SolidPropertiesTestApp__registerApps()
{
  SolidPropertiesTestApp::registerApps();
}
void
SolidPropertiesTestApp::registerApps()
{
  registerApp(SolidPropertiesApp);
  registerApp(SolidPropertiesTestApp);
}

// External entry point for dynamic object registration
extern "C" void
SolidPropertiesTestApp__registerObjects(Factory & factory)
{
  SolidPropertiesTestApp::registerObjects(factory);
}
void
SolidPropertiesTestApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"SolidPropertiesTestApp"});
}

// External entry point for dynamic syntax association
extern "C" void
SolidPropertiesTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  SolidPropertiesTestApp::associateSyntax(syntax, action_factory);
}
void
SolidPropertiesTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"SolidPropertiesTestApp"});
}

// External entry point for dynamic execute flag registration
extern "C" void
SolidPropertiesTestApp__registerExecFlags(Factory & factory)
{
  SolidPropertiesTestApp::registerExecFlags(factory);
}
void
SolidPropertiesTestApp::registerExecFlags(Factory & /*factory*/)
{
}
