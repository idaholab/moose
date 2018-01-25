//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "PorousFlowTestApp.h"
#include "PorousFlowApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<PorousFlowTestApp>()
{
  InputParameters params = validParams<PorousFlowApp>();
  return params;
}

PorousFlowTestApp::PorousFlowTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  PorousFlowApp::registerObjectDepends(_factory);
  PorousFlowApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  PorousFlowApp::associateSyntaxDepends(_syntax, _action_factory);
  PorousFlowApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  PorousFlowApp::registerExecFlags(_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    PorousFlowTestApp::registerObjects(_factory);
    PorousFlowTestApp::associateSyntax(_syntax, _action_factory);
  }
}

PorousFlowTestApp::~PorousFlowTestApp() {}

// External entry point for dynamic application loading
extern "C" void
PorousFlowTestApp__registerApps()
{
  PorousFlowTestApp::registerApps();
}
void
PorousFlowTestApp::registerApps()
{
  registerApp(PorousFlowApp);
  registerApp(PorousFlowTestApp);
}

// External entry point for dynamic object registration
extern "C" void
PorousFlowTestApp__registerObjects(Factory & factory)
{
  PorousFlowTestApp::registerObjects(factory);
}
void
PorousFlowTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
PorousFlowTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  PorousFlowTestApp::associateSyntax(syntax, action_factory);
}
void
PorousFlowTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

// External entry point for dynamic execute flag registration
extern "C" void
PorousFlowTestApp__registerExecFlags(Factory & factory)
{
  PorousFlowTestApp::registerExecFlags(factory);
}
void
PorousFlowTestApp::registerExecFlags(Factory & /*factory*/)
{
}
