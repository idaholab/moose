//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsTestApp.h"
#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
TensorMechanicsTestApp::validParams()
{
  InputParameters params = TensorMechanicsApp::validParams();
  return params;
}

registerKnownLabel("TensorMechanicsTestApp");

TensorMechanicsTestApp::TensorMechanicsTestApp(InputParameters parameters) : MooseApp(parameters)
{
  TensorMechanicsTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

TensorMechanicsTestApp::~TensorMechanicsTestApp() {}

void
TensorMechanicsTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  TensorMechanicsApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"TensorMechanicsTestApp"});
    Registry::registerActionsTo(af, {"TensorMechanicsTestApp"});
  }
}
void
TensorMechanicsTestApp::registerApps()
{
  registerApp(TensorMechanicsApp);
  registerApp(TensorMechanicsTestApp);
}

void
TensorMechanicsTestApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"TensorMechanicsTestApp"});
}

void
TensorMechanicsTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"TensorMechanicsTestApp"});
}

void
TensorMechanicsTestApp::registerExecFlags(Factory & /*factory*/)
{
}

extern "C" void
TensorMechanicsTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  TensorMechanicsTestApp::registerAll(f, af, s);
}
extern "C" void
TensorMechanicsTestApp__registerApps()
{
  TensorMechanicsTestApp::registerApps();
}
