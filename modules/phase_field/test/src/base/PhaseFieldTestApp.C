//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldTestApp.h"
#include "PhaseFieldApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "GaussContForcing.h"

InputParameters
PhaseFieldTestApp::validParams()
{
  InputParameters params = PhaseFieldApp::validParams();
  return params;
}

registerKnownLabel("PhaseFieldTestApp");

PhaseFieldTestApp::PhaseFieldTestApp(InputParameters parameters) : MooseApp(parameters)
{
  PhaseFieldTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

PhaseFieldTestApp::~PhaseFieldTestApp() {}

void
PhaseFieldTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  PhaseFieldApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"PhaseFieldTestApp"});
    Registry::registerActionsTo(af, {"PhaseFieldTestApp"});
  }
}

void
PhaseFieldTestApp::registerApps()
{
  registerApp(PhaseFieldApp);
  registerApp(PhaseFieldTestApp);
}

void
PhaseFieldTestApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"PhaseFieldTestApp"});
}

void
PhaseFieldTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"PhaseFieldTestApp"});
}

void
PhaseFieldTestApp::registerExecFlags(Factory & /*factory*/)
{
}

extern "C" void
PhaseFieldTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  PhaseFieldTestApp::registerAll(f, af, s);
}
extern "C" void
PhaseFieldTestApp__registerApps()
{
  PhaseFieldTestApp::registerApps();
}
