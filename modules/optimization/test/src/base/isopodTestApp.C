//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "isopodTestApp.h"
#include "isopodApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

InputParameters
isopodTestApp::validParams()
{
  InputParameters params = isopodApp::validParams();
  return params;
}

isopodTestApp::isopodTestApp(InputParameters parameters) : MooseApp(parameters)
{
  isopodTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

isopodTestApp::~isopodTestApp() {}

void
isopodTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  isopodApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"isopodTestApp"});
    Registry::registerActionsTo(af, {"isopodTestApp"});
  }
}

void
isopodTestApp::registerApps()
{
  registerApp(isopodApp);
  registerApp(isopodTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
isopodTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  isopodTestApp::registerAll(f, af, s);
}
extern "C" void
isopodTestApp__registerApps()
{
  isopodTestApp::registerApps();
}
