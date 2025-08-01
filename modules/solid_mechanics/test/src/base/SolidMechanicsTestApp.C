//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMechanicsTestApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
SolidMechanicsTestApp::validParams()
{
  InputParameters params = SolidMechanicsApp::validParams();
  return params;
}

registerKnownLabel("SolidMechanicsTestApp");

SolidMechanicsTestApp::SolidMechanicsTestApp(const InputParameters & parameters)
  : SolidMechanicsApp(parameters)
{
  SolidMechanicsTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

SolidMechanicsTestApp::~SolidMechanicsTestApp() {}

void
SolidMechanicsTestApp::registerAll(Factory & f,
                                   ActionFactory & af,
                                   Syntax & /*s*/,
                                   bool use_test_objs)
{
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"SolidMechanicsTestApp"});
    Registry::registerActionsTo(af, {"SolidMechanicsTestApp"});
  }
}
void
SolidMechanicsTestApp::registerApps()
{
  SolidMechanicsApp::registerApps();
  registerApp(SolidMechanicsTestApp);
}

extern "C" void
SolidMechanicsTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SolidMechanicsTestApp::registerAll(f, af, s);
}
extern "C" void
SolidMechanicsTestApp__registerApps()
{
  SolidMechanicsTestApp::registerApps();
}
