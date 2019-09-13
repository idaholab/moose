//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "sub_channelTestApp.h"
#include "sub_channelApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

template <>
InputParameters
validParams<sub_channelTestApp>()
{
  InputParameters params = validParams<sub_channelApp>();
  return params;
}

sub_channelTestApp::sub_channelTestApp(InputParameters parameters) : MooseApp(parameters)
{
  sub_channelTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

sub_channelTestApp::~sub_channelTestApp() {}

void
sub_channelTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  sub_channelApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"sub_channelTestApp"});
    Registry::registerActionsTo(af, {"sub_channelTestApp"});
  }
}

void
sub_channelTestApp::registerApps()
{
  registerApp(sub_channelApp);
  registerApp(sub_channelTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
sub_channelTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  sub_channelTestApp::registerAll(f, af, s);
}
extern "C" void
sub_channelTestApp__registerApps()
{
  sub_channelTestApp::registerApps();
}
