//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

ContactTestApp::ContactTestApp(const InputParameters & parameters) : MooseApp(parameters)
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
  ContactApp::registerApps();
  registerApp(ContactTestApp);
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
