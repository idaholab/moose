//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "FussionAppTestApp.h"
#include "FussionAppApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

InputParameters
FussionAppTestApp::validParams()
{
  InputParameters params = FussionAppApp::validParams();
  return params;
}

FussionAppTestApp::FussionAppTestApp(InputParameters parameters) : MooseApp(parameters)
{
  FussionAppTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

FussionAppTestApp::~FussionAppTestApp() {}

void
FussionAppTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  FussionAppApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"FussionAppTestApp"});
    Registry::registerActionsTo(af, {"FussionAppTestApp"});
  }
}

void
FussionAppTestApp::registerApps()
{
  registerApp(FussionAppApp);
  registerApp(FussionAppTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
FussionAppTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FussionAppTestApp::registerAll(f, af, s);
}
extern "C" void
FussionAppTestApp__registerApps()
{
  FussionAppTestApp::registerApps();
}
