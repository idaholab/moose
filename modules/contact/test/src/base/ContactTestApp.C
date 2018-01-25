//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "ContactTestApp.h"
#include "ContactApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<ContactTestApp>()
{
  InputParameters params = validParams<ContactApp>();
  return params;
}

ContactTestApp::ContactTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ContactApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ContactApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  ContactApp::registerExecFlags(_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    ContactTestApp::registerObjects(_factory);
    ContactTestApp::associateSyntax(_syntax, _action_factory);
  }
}

ContactTestApp::~ContactTestApp() {}

// External entry point for dynamic application loading
extern "C" void
ContactTestApp__registerApps()
{
  ContactTestApp::registerApps();
}
void
ContactTestApp::registerApps()
{
  registerApp(ContactApp);
  registerApp(ContactTestApp);
}

// External entry point for dynamic object registration
extern "C" void
ContactTestApp__registerObjects(Factory & factory)
{
  ContactTestApp::registerObjects(factory);
}
void
ContactTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
ContactTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  ContactTestApp::associateSyntax(syntax, action_factory);
}
void
ContactTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

// External entry point for dynamic execute flag registration
extern "C" void
ContactTestApp__registerExecFlags(Factory & factory)
{
  ContactTestApp::registerExecFlags(factory);
}
void
ContactTestApp::registerExecFlags(Factory & /*factory*/)
{
}
