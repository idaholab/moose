//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMechanicsApp.h"
#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<SolidMechanicsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("SolidMechanicsApp");

SolidMechanicsApp::SolidMechanicsApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  SolidMechanicsApp::registerObjectDepends(_factory);
  SolidMechanicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  SolidMechanicsApp::associateSyntaxDepends(_syntax, _action_factory);
  SolidMechanicsApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  SolidMechanicsApp::registerExecFlags(_factory);
}

SolidMechanicsApp::~SolidMechanicsApp() {}

// External entry point for dynamic application loading
extern "C" void
SolidMechanicsApp__registerApps()
{
  SolidMechanicsApp::registerApps();
}
void
SolidMechanicsApp::registerApps()
{
  registerApp(SolidMechanicsApp);
}

void
SolidMechanicsApp::registerObjectDepends(Factory & factory)
{
  TensorMechanicsApp::registerObjects(factory);
}

// External entry point for dynamic object registration
extern "C" void
SolidMechanicsApp__registerObjects(Factory & factory)
{
  TensorMechanicsApp::registerObjects(factory);
}
void
SolidMechanicsApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"SolidMechanicsApp"});
}

void
SolidMechanicsApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
}

// External entry point for dynamic syntax association
extern "C" void
SolidMechanicsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  SolidMechanicsApp::associateSyntax(syntax, action_factory);
}
void
SolidMechanicsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"SolidMechanicsApp"});
  registerSyntax("SolidMechanicsAction", "SolidMechanics/*");
}

// External entry point for dynamic execute flag registration
extern "C" void
SolidMechanicsApp__registerExecFlags(Factory & factory)
{
  SolidMechanicsApp::registerExecFlags(factory);
}
void
SolidMechanicsApp::registerExecFlags(Factory & /*factory*/)
{
}
