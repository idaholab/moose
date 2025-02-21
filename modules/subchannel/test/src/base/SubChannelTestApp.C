//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubChannelTestApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
SubChannelTestApp::validParams()
{
  InputParameters params = SubChannelApp::validParams();
  return params;
}

SubChannelTestApp::SubChannelTestApp(InputParameters parameters) : SubChannelApp(parameters)
{
  SubChannelTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

SubChannelTestApp::~SubChannelTestApp() {}

void
SubChannelTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  SubChannelApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"SubChannelTestApp"});
    Registry::registerActionsTo(af, {"SubChannelTestApp"});
  }
}

void
SubChannelTestApp::registerApps()
{
  registerApp(SubChannelTestApp);
  SubChannelApp::registerApps();
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
SubChannelTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SubChannelTestApp::registerAll(f, af, s);
}
extern "C" void
SubChannelTestApp__registerApps()
{
  SubChannelTestApp::registerApps();
}
