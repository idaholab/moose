//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMTestApp.h"
#include "XFEMAppTypes.h"
#include "XFEMApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
XFEMTestApp::validParams()
{
  InputParameters params = XFEMApp::validParams();
  return params;
}

registerKnownLabel("XFEMTestApp");

XFEMTestApp::XFEMTestApp(const InputParameters & parameters) : MooseApp(parameters)
{
  XFEMTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

XFEMTestApp::~XFEMTestApp() {}

void
XFEMTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  XFEMApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"XFEMTestApp"});
    Registry::registerActionsTo(af, {"XFEMTestApp"});
  }
}

void
XFEMTestApp::registerApps()
{
  XFEMApp::registerApps();
  registerApp(XFEMTestApp);
}

extern "C" void
XFEMTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  XFEMTestApp::registerAll(f, af, s);
}
extern "C" void
XFEMTestApp__registerApps()
{
  XFEMTestApp::registerApps();
}
