//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMechanicsApp.h"
#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<SolidMechanicsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("SolidMechanicsApp");

SolidMechanicsApp::SolidMechanicsApp(const InputParameters & parameters) : MooseApp(parameters)
{
  SolidMechanicsApp::registerAll(_factory, _action_factory, _syntax);
}

SolidMechanicsApp::~SolidMechanicsApp() {}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerSyntax("SolidMechanicsAction", "SolidMechanics/*");
}

void
SolidMechanicsApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"SolidMechanicsApp"});
  Registry::registerActionsTo(af, {"SolidMechanicsApp"});
  associateSyntaxInner(s, af);

  TensorMechanicsApp::registerAll(f, af, s);
}

void
SolidMechanicsApp::registerApps()
{
  registerApp(SolidMechanicsApp);
}

void
SolidMechanicsApp::registerObjectDepends(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjectsDepends");
  TensorMechanicsApp::registerObjects(factory);
}

void
SolidMechanicsApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"SolidMechanicsApp"});
}

void
SolidMechanicsApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntaxDepends");
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
}

void
SolidMechanicsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"SolidMechanicsApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
SolidMechanicsApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("use registerAll instead of registerExecFlags");
}

extern "C" void
SolidMechanicsApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SolidMechanicsApp::registerAll(f, af, s);
}
extern "C" void
SolidMechanicsApp__registerApps()
{
  SolidMechanicsApp::registerApps();
}
