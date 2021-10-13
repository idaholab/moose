//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "ReactorTestApp.h"
#include "ReactorApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ReactorTestApp::validParams()
{
  InputParameters params = ReactorApp::validParams();
  return params;
}

ReactorTestApp::ReactorTestApp(InputParameters parameters) : MooseApp(parameters)
{
  ReactorTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

ReactorTestApp::~ReactorTestApp() {}

void
ReactorTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  ReactorApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"ReactorTestApp"});
    Registry::registerActionsTo(af, {"ReactorTestApp"});
  }
}

void
ReactorTestApp::registerApps()
{
  registerApp(ReactorApp);
  registerApp(ReactorTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
ReactorTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ReactorTestApp::registerAll(f, af, s);
}
extern "C" void
ReactorTestApp__registerApps()
{
  ReactorTestApp::registerApps();
}
