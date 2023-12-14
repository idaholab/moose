//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "SolidPropertiesTestApp.h"
#include "SolidPropertiesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
SolidPropertiesTestApp::validParams()
{
  InputParameters params = SolidPropertiesApp::validParams();
  return params;
}

SolidPropertiesTestApp::SolidPropertiesTestApp(InputParameters parameters) : MooseApp(parameters)
{
  SolidPropertiesTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

SolidPropertiesTestApp::~SolidPropertiesTestApp() {}

void
SolidPropertiesTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  SolidPropertiesApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"SolidPropertiesTestApp"});
    Registry::registerActionsTo(af, {"SolidPropertiesTestApp"});
  }
}

void
SolidPropertiesTestApp::registerApps()
{
  registerApp(SolidPropertiesApp);
  registerApp(SolidPropertiesTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
SolidPropertiesTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SolidPropertiesTestApp::registerAll(f, af, s);
}
extern "C" void
SolidPropertiesTestApp__registerApps()
{
  SolidPropertiesTestApp::registerApps();
}
