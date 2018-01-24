//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "RdgTestApp.h"
#include "RdgApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<RdgTestApp>()
{
  InputParameters params = validParams<RdgApp>();
  return params;
}

RdgTestApp::RdgTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  RdgApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  RdgApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  RdgApp::registerExecFlags(_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    RdgTestApp::registerObjects(_factory);
    RdgTestApp::associateSyntax(_syntax, _action_factory);
  }
}

RdgTestApp::~RdgTestApp() {}

// External entry point for dynamic application loading
extern "C" void
RdgTestApp__registerApps()
{
  RdgTestApp::registerApps();
}
void
RdgTestApp::registerApps()
{
  registerApp(RdgApp);
  registerApp(RdgTestApp);
}

// External entry point for dynamic object registration
extern "C" void
RdgTestApp__registerObjects(Factory & factory)
{
  RdgTestApp::registerObjects(factory);
}
void
RdgTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
RdgTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  RdgTestApp::associateSyntax(syntax, action_factory);
}
void
RdgTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

// External entry point for dynamic execute flag registration
extern "C" void
RdgTestApp__registerExecFlags(Factory & factory)
{
  RdgTestApp::registerExecFlags(factory);
}
void
RdgTestApp::registerExecFlags(Factory & /*factory*/)
{
}
