//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "FunctionalExpansionToolsTestApp.h"
#include "FunctionalExpansionToolsApp.h"

InputParameters
FunctionalExpansionToolsTestApp::validParams()
{
  InputParameters params = FunctionalExpansionToolsApp::validParams();
  return params;
}

registerKnownLabel("FunctionalExpansionToolsTestApp");

FunctionalExpansionToolsTestApp::FunctionalExpansionToolsTestApp(const InputParameters & parameters)
  : MooseApp(parameters)
{
  FunctionalExpansionToolsTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

void
FunctionalExpansionToolsTestApp::registerAll(Factory & f,
                                             ActionFactory & af,
                                             Syntax & s,
                                             bool use_test_objs)
{
  FunctionalExpansionToolsApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"FunctionalExpansionToolsTestApp"});
    Registry::registerActionsTo(af, {"FunctionalExpansionToolsTestApp"});
  }
}

FunctionalExpansionToolsTestApp::~FunctionalExpansionToolsTestApp() {}

void
FunctionalExpansionToolsTestApp::registerApps()
{
  FunctionalExpansionToolsApp::registerApps();
  registerApp(FunctionalExpansionToolsTestApp);
}

extern "C" void
FunctionalExpansionToolsTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FunctionalExpansionToolsTestApp::registerAll(f, af, s);
}
extern "C" void
FunctionalExpansionToolsTestApp__registerApps()
{
  FunctionalExpansionToolsTestApp::registerApps();
}
