//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsTestApp.h"
#include "RichardsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
RichardsTestApp::validParams()
{
  InputParameters params = RichardsApp::validParams();
  return params;
}

registerKnownLabel("RichardsTestApp");

RichardsTestApp::RichardsTestApp(InputParameters parameters) : MooseApp(parameters)
{
  mooseDeprecated("Please use the PorousFlow module instead.  If Richards contains functionality "
                  "not included in PorousFlow, please contact the moose-users google group");
  RichardsTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

RichardsTestApp::~RichardsTestApp() {}

void
RichardsTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  RichardsApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"RichardsTestApp"});
    Registry::registerActionsTo(af, {"RichardsTestApp"});
  }
}

void
RichardsTestApp::registerApps()
{
  registerApp(RichardsApp);
  registerApp(RichardsTestApp);
}

void
RichardsTestApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"RichardsTestApp"});
}

void
RichardsTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"RichardsTestApp"});
}

void
RichardsTestApp::registerExecFlags(Factory & /*factory*/)
{
}

extern "C" void
RichardsTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  RichardsTestApp::registerAll(f, af, s);
}
extern "C" void
RichardsTestApp__registerApps()
{
  RichardsTestApp::registerApps();
}
