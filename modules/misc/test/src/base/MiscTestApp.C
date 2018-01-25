//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "MiscTestApp.h"
#include "MiscApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "Convection.h"

template <>
InputParameters
validParams<MiscTestApp>()
{
  InputParameters params = validParams<MiscApp>();
  return params;
}

MiscTestApp::MiscTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  MiscApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  MiscApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  MiscApp::registerExecFlags(_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    MiscTestApp::registerObjects(_factory);
    MiscTestApp::associateSyntax(_syntax, _action_factory);
    MiscTestApp::registerExecFlags(_factory);
  }
}

MiscTestApp::~MiscTestApp() {}

// External entry point for dynamic application loading
extern "C" void
MiscTestApp__registerApps()
{
  MiscTestApp::registerApps();
}
void
MiscTestApp::registerApps()
{
  registerApp(MiscApp);
  registerApp(MiscTestApp);
}

// External entry point for dynamic object registration
extern "C" void
MiscTestApp__registerObjects(Factory & factory)
{
  MiscTestApp::registerObjects(factory);
}
void
MiscTestApp::registerObjects(Factory & factory)
{
  registerKernel(Convection);
}

// External entry point for dynamic syntax association
extern "C" void
MiscTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  MiscTestApp::associateSyntax(syntax, action_factory);
}
void
MiscTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

// External entry point for dynamic execute flag registration
extern "C" void
MiscTestApp__registerExecFlags(Factory & factory)
{
  MiscTestApp::registerExecFlags(factory);
}
void
MiscTestApp::registerExecFlags(Factory & /*factory*/)
{
}
