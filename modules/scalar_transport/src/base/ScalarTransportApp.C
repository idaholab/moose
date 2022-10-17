//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarTransportApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ScalarTransportApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("automatic_automatic_scaling") = false;
  return params;
}

ScalarTransportApp::ScalarTransportApp(InputParameters parameters) : MooseApp(parameters)
{
  ScalarTransportApp::registerAll(_factory, _action_factory, _syntax);
}

ScalarTransportApp::~ScalarTransportApp() {}

void
ScalarTransportApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  /* ModulesApp::registerAll(f, af, s); */
  Registry::registerObjectsTo(f, {"ScalarTransportApp"});
  Registry::registerActionsTo(af, {"ScalarTransportApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
ScalarTransportApp::registerApps()
{
  registerApp(ScalarTransportApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
ScalarTransportApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ScalarTransportApp::registerAll(f, af, s);
}
extern "C" void
ScalarTransportApp__registerApps()
{
  ScalarTransportApp::registerApps();
}
