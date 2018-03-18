//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RdgApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<RdgApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("RdgApp");

RdgApp::RdgApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  RdgApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  RdgApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  RdgApp::registerExecFlags(_factory);
}

RdgApp::~RdgApp() {}

// External entry point for dynamic application loading
extern "C" void
RdgApp__registerApps()
{
  RdgApp::registerApps();
}
void
RdgApp::registerApps()
{
  registerApp(RdgApp);
}

// External entry point for dynamic object registration
extern "C" void
RdgApp__registerObjects(Factory & factory)
{
  RdgApp::registerObjects(factory);
}
void
RdgApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"RdgApp"});
}

// External entry point for dynamic syntax association
extern "C" void
RdgApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  RdgApp::associateSyntax(syntax, action_factory);
}
void
RdgApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"RdgApp"});
}

// External entry point for dynamic execute flag registration
extern "C" void
RdgApp__registerExecFlags(Factory & factory)
{
  RdgApp::registerExecFlags(factory);
}
void
RdgApp::registerExecFlags(Factory & /*factory*/)
{
}
