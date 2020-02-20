//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "geochemistryTestApp.h"
#include "geochemistryApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
geochemistryTestApp::validParams()
{
  InputParameters params = geochemistryApp::validParams();
  return params;
}

geochemistryTestApp::geochemistryTestApp(InputParameters parameters) : MooseApp(parameters)
{
  geochemistryTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

geochemistryTestApp::~geochemistryTestApp() {}

void
geochemistryTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  geochemistryApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"geochemistryTestApp"});
    Registry::registerActionsTo(af, {"geochemistryTestApp"});
  }
}

void
geochemistryTestApp::registerApps()
{
  registerApp(geochemistryApp);
  registerApp(geochemistryTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
geochemistryTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  geochemistryTestApp::registerAll(f, af, s);
}
extern "C" void
geochemistryTestApp__registerApps()
{
  geochemistryTestApp::registerApps();
}
