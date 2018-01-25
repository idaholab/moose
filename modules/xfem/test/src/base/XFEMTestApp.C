//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "XFEMTestApp.h"
#include "XFEMApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<XFEMTestApp>()
{
  InputParameters params = validParams<XFEMApp>();
  return params;
}

XFEMTestApp::XFEMTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  XFEMApp::registerObjectDepends(_factory);
  XFEMApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  XFEMApp::associateSyntaxDepends(_syntax, _action_factory);
  XFEMApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  XFEMApp::registerExecFlags(_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    XFEMTestApp::registerObjects(_factory);
    XFEMTestApp::associateSyntax(_syntax, _action_factory);
  }
}

XFEMTestApp::~XFEMTestApp() {}

// External entry point for dynamic application loading
extern "C" void
XFEMTestApp__registerApps()
{
  XFEMTestApp::registerApps();
}
void
XFEMTestApp::registerApps()
{
  registerApp(XFEMApp);
  registerApp(XFEMTestApp);
}

// External entry point for dynamic object registration
extern "C" void
XFEMTestApp__registerObjects(Factory & factory)
{
  XFEMTestApp::registerObjects(factory);
}
void
XFEMTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
XFEMTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  XFEMTestApp::associateSyntax(syntax, action_factory);
}
void
XFEMTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

// External entry point for dynamic execute flag registration
extern "C" void
XFEMTestApp__registerExecFlags(Factory & factory)
{
  XFEMTestApp::registerExecFlags(factory);
}
void
XFEMTestApp::registerExecFlags(Factory & /*factory*/)
{
}
