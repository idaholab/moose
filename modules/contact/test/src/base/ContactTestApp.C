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

InputParameters
ContactTestApp::validParams()
{
  InputParameters params = ContactApp::validParams();
  return params;
}

registerKnownLabel("ContactTestApp");

ContactTestApp::ContactTestApp(InputParameters parameters) : MooseApp(parameters)
{
  ContactTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

ContactTestApp::~ContactTestApp() {}

void
ContactTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  ContactApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"ContactTestApp"});
    Registry::registerActionsTo(af, {"ContactTestApp"});
  }
}

void
ContactTestApp::registerApps()
{
  registerApp(ContactApp);
  registerApp(ContactTestApp);
}

void
ContactTestApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"ContactTestApp"});
}

void
ContactTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"ContactTestApp"});
}

void
ContactTestApp::registerExecFlags(Factory & /*factory*/)
{
}

extern "C" void
ContactTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ContactTestApp::registerAll(f, af, s);
}
extern "C" void
ContactTestApp__registerApps()
{
  ContactTestApp::registerApps();
}
