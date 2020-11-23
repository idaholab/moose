//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "BabblerTestApp.h"
#include "BabblerApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

InputParameters
BabblerTestApp::validParams()
{
  InputParameters params = BabblerApp::validParams();
  return params;
}

BabblerTestApp::BabblerTestApp(InputParameters parameters) : MooseApp(parameters)
{
  BabblerTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

BabblerTestApp::~BabblerTestApp() {}

void
BabblerTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  BabblerApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"BabblerTestApp"});
    Registry::registerActionsTo(af, {"BabblerTestApp"});
  }
}

void
BabblerTestApp::registerApps()
{
  registerApp(BabblerApp);
  registerApp(BabblerTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
BabblerTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  BabblerTestApp::registerAll(f, af, s);
}
extern "C" void
BabblerTestApp__registerApps()
{
  BabblerTestApp::registerApps();
}
