//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeridynamicsApp.h"
#include "TensorMechanicsApp.h" // tensor mechanics dependency
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<PeridynamicsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("PeridynamicsApp");

PeridynamicsApp::PeridynamicsApp(const InputParameters & parameters) : MooseApp(parameters)
{
  PeridynamicsApp::registerAll(_factory, _action_factory, _syntax);
}

PeridynamicsApp::~PeridynamicsApp() {}

void
PeridynamicsApp::registerApps()
{
  registerApp(PeridynamicsApp);
}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerSyntax("MechanicsActionPD", "Modules/Peridynamics/Mechanics/Master/*");
  registerSyntax("GeneralizedPlaneStrainActionPD",
                 "Modules/Peridynamics/Mechanics/GeneralizedPlaneStrain/*");
}

void
PeridynamicsApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"PeridynamicsApp"});
  Registry::registerActionsTo(af, {"PeridynamicsApp"});
  associateSyntaxInner(s, af);

  TensorMechanicsApp::registerAll(f, af, s);
}

extern "C" void
PeridynamicsApp__registerApps()
{
  PeridynamicsApp::registerApps();
}

extern "C" void
PeridynamicsApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  PeridynamicsApp::registerAll(f, af, s);
}
