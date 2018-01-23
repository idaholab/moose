//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "StorkTestApp.h"
#include "StorkApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

template <>
InputParameters
validParams<StorkTestApp>()
{
  InputParameters params = validParams<StorkApp>();
  return params;
}

StorkTestApp::StorkTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);
  StorkApp::registerObjectDepends(_factory);
  StorkApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
  StorkApp::associateSyntaxDepends(_syntax, _action_factory);
  StorkApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  ModulesApp::registerExecFlags(_factory);
  StorkApp::registerExecFlags(_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    StorkTestApp::registerObjects(_factory);
    StorkTestApp::associateSyntax(_syntax, _action_factory);
    StorkTestApp::registerExecFlags(_factory);
  }
}

StorkTestApp::~StorkTestApp() {}

void
StorkTestApp::registerApps()
{
  registerApp(StorkApp);
  registerApp(StorkTestApp);
}

void
StorkTestApp::registerObjects(Factory & /*factory*/)
{
  /* Uncomment Factory parameter and register your new test objects here! */
}

void
StorkTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
  /* Uncomment Syntax and ActionFactory parameters and register your new test objects here! */
}

void
StorkTestApp::registerExecFlags(Factory & /*factory*/)
{
  /* Uncomment Factory parameter and register your new execute flags here! */
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
StorkTestApp__registerApps()
{
  StorkTestApp::registerApps();
}

// External entry point for dynamic object registration
extern "C" void
StorkTestApp__registerObjects(Factory & factory)
{
  StorkTestApp::registerObjects(factory);
}

// External entry point for dynamic syntax association
extern "C" void
StorkTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  StorkTestApp::associateSyntax(syntax, action_factory);
}

// External entry point for dynamic execute flag loading
extern "C" void
StorkTestApp__registerExecFlags(Factory & factory)
{
  StorkTestApp::registerExecFlags(factory);
}
