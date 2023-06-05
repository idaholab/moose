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

InputParameters
StorkTestApp::validParams()
{
  InputParameters params = StorkApp::validParams();
  params.set<bool>("use_legacy_material_output") = false;
  return params;
}

StorkTestApp::StorkTestApp(InputParameters parameters) : MooseApp(parameters)
{
  StorkTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

StorkTestApp::~StorkTestApp() {}

void
StorkTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  StorkApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"StorkTestApp"});
    Registry::registerActionsTo(af, {"StorkTestApp"});
  }
}

void
StorkTestApp::registerApps()
{
  registerApp(StorkApp);
  registerApp(StorkTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
StorkTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  StorkTestApp::registerAll(f, af, s);
}
extern "C" void
StorkTestApp__registerApps()
{
  StorkTestApp::registerApps();
}
