//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "GeochemistryTestApp.h"
#include "GeochemistryApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
GeochemistryTestApp::validParams()
{
  InputParameters params = GeochemistryApp::validParams();
  return params;
}

GeochemistryTestApp::GeochemistryTestApp(InputParameters parameters) : MooseApp(parameters)
{
  GeochemistryTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

GeochemistryTestApp::~GeochemistryTestApp() {}

void
GeochemistryTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  GeochemistryApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"GeochemistryTestApp"});
    Registry::registerActionsTo(af, {"GeochemistryTestApp"});
  }
}

void
GeochemistryTestApp::registerApps()
{
  registerApp(GeochemistryApp);
  registerApp(GeochemistryTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
GeochemistryTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  GeochemistryTestApp::registerAll(f, af, s);
}
extern "C" void
GeochemistryTestApp__registerApps()
{
  GeochemistryTestApp::registerApps();
}
