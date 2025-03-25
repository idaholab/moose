//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatTransferTestApp.h"
#include "HeatTransferApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
HeatTransferTestApp::validParams()
{
  InputParameters params = HeatTransferApp::validParams();
  return params;
}

registerKnownLabel("HeatTransferTestApp");

HeatTransferTestApp::HeatTransferTestApp(InputParameters parameters) : MooseApp(parameters)
{
  HeatTransferTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

HeatTransferTestApp::~HeatTransferTestApp() {}

void
HeatTransferTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  HeatTransferApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"HeatTransferTestApp"});
    Registry::registerActionsTo(af, {"HeatTransferTestApp"});
  }
}

void
HeatTransferTestApp::registerApps()
{
  registerApp(HeatTransferApp);
  registerApp(HeatTransferTestApp);
}

void
HeatTransferTestApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"HeatTransferTestApp"});
}

void
HeatTransferTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"HeatTransferTestApp"});
}

void
HeatTransferTestApp::registerExecFlags(Factory & /*factory*/)
{
}

extern "C" void
HeatTransferTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  HeatTransferTestApp::registerAll(f, af, s);
}
extern "C" void
HeatTransferTestApp__registerApps()
{
  HeatTransferTestApp::registerApps();
}
