//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "SolidMechanicsTestApp.h"
#include "SolidMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<SolidMechanicsTestApp>()
{
  InputParameters params = validParams<SolidMechanicsApp>();
  return params;
}

SolidMechanicsTestApp::SolidMechanicsTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  SolidMechanicsApp::registerObjectDepends(_factory);
  SolidMechanicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  SolidMechanicsApp::associateSyntaxDepends(_syntax, _action_factory);
  SolidMechanicsApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  SolidMechanicsApp::registerExecFlags(_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    SolidMechanicsTestApp::registerObjects(_factory);
    SolidMechanicsTestApp::associateSyntax(_syntax, _action_factory);
  }
}

SolidMechanicsTestApp::~SolidMechanicsTestApp() {}

// External entry point for dynamic application loading
extern "C" void
SolidMechanicsTestApp__registerApps()
{
  SolidMechanicsTestApp::registerApps();
}
void
SolidMechanicsTestApp::registerApps()
{
  registerApp(SolidMechanicsApp);
  registerApp(SolidMechanicsTestApp);
}

// External entry point for dynamic object registration
extern "C" void
SolidMechanicsTestApp__registerObjects(Factory & factory)
{
  SolidMechanicsTestApp::registerObjects(factory);
}
void
SolidMechanicsTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
SolidMechanicsTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  SolidMechanicsTestApp::associateSyntax(syntax, action_factory);
}
void
SolidMechanicsTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

// External entry point for dynamic execute flag registration
extern "C" void
SolidMechanicsTestApp__registerExecFlags(Factory & factory)
{
  SolidMechanicsTestApp::registerExecFlags(factory);
}
void
SolidMechanicsTestApp::registerExecFlags(Factory & /*factory*/)
{
}
