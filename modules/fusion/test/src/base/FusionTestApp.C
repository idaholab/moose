//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FusionTestApp.h"
#include "FusionApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
FusionTestApp::validParams()
{
  InputParameters params = FusionApp::validParams();
  return params;
}

FusionTestApp::FusionTestApp(InputParameters parameters) : MooseApp(parameters)
{
  FusionTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

FusionTestApp::~FusionTestApp() {}

void
FusionTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  FusionApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"FusionTestApp"});
    Registry::registerActionsTo(af, {"FusionTestApp"});
  }
}

void
FusionTestApp::registerApps()
{
  registerApp(FusionApp);
  registerApp(FusionTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
FusionTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FusionTestApp::registerAll(f, af, s);
}
extern "C" void
FusionTestApp__registerApps()
{
  FusionTestApp::registerApps();
}
