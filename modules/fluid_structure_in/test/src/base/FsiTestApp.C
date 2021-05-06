//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "FsiTestApp.h"
#include "FsiApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
FsiTestApp::validParams()
{
  InputParameters params = FsiApp::validParams();
  return params;
}

FsiTestApp::FsiTestApp(InputParameters parameters) : MooseApp(parameters)
{
  FsiTestApp::registerAll(_factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

FsiTestApp::~FsiTestApp() {}

void
FsiTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  FsiApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"FsiTestApp"});
    Registry::registerActionsTo(af, {"FsiTestApp"});
  }
}

void
FsiTestApp::registerApps()
{
  registerApp(FsiApp);
  registerApp(FsiTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
FsiTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FsiTestApp::registerAll(f, af, s);
}
extern "C" void
FsiTestApp__registerApps()
{
  FsiTestApp::registerApps();
}
