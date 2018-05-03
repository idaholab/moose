//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "PeridynamicsTestApp.h"
#include "PeridynamicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<PeridynamicsTestApp>()
{
  InputParameters params = validParams<PeridynamicsApp>();
  return params;
}

PeridynamicsTestApp::PeridynamicsTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  PeridynamicsApp::registerObjectDepends(_factory);
  PeridynamicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  PeridynamicsApp::associateSyntaxDepends(_syntax, _action_factory);
  PeridynamicsApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  PeridynamicsApp::registerExecFlags(_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    PeridynamicsTestApp::registerObjects(_factory);
    PeridynamicsTestApp::associateSyntax(_syntax, _action_factory);
    PeridynamicsTestApp::registerExecFlags(_factory);
  }
}

PeridynamicsTestApp::~PeridynamicsTestApp() {}

void
PeridynamicsTestApp::registerApps()
{
  registerApp(PeridynamicsApp);
  registerApp(PeridynamicsTestApp);
}

void
PeridynamicsTestApp::registerObjects(Factory & /*factory*/)
{
  /* Uncomment Factory parameter and register your new test objects here! */
}

void
PeridynamicsTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
  /* Uncomment Syntax and ActionFactory parameters and register your new test objects here! */
}

void
PeridynamicsTestApp::registerExecFlags(Factory & /*factory*/)
{
  /* Uncomment Factory parameter and register your new execute flags here! */
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
PeridynamicsTestApp__registerApps()
{
  PeridynamicsTestApp::registerApps();
}

// External entry point for dynamic object registration
extern "C" void
PeridynamicsTestApp__registerObjects(Factory & factory)
{
  PeridynamicsTestApp::registerObjects(factory);
}

// External entry point for dynamic syntax association
extern "C" void
PeridynamicsTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  PeridynamicsTestApp::associateSyntax(syntax, action_factory);
}

// External entry point for dynamic execute flag registration
extern "C" void
PeridynamicsTestApp__registerExecFlags(Factory & factory)
{
  PeridynamicsTestApp::registerExecFlags(factory);
}
