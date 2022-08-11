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

InputParameters
PeridynamicsApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;

  params.set<bool>("use_legacy_material_output") = false;

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

void
PeridynamicsApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"PeridynamicsApp"});
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

void
PeridynamicsApp::registerObjectDepends(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjectsDepends");
  TensorMechanicsApp::registerObjects(factory);
}

void
PeridynamicsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"PeridynamicsApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
PeridynamicsApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of registerObjectsDepends");
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
}

void
PeridynamicsApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
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
