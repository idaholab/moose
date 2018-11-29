//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<SolidPropertiesApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("SolidPropertiesApp");

SolidPropertiesApp::SolidPropertiesApp(InputParameters parameters) : MooseApp(parameters)
{
  SolidPropertiesApp::registerAll(_factory, _action_factory, _syntax);
}

SolidPropertiesApp::~SolidPropertiesApp() {}

void
SolidPropertiesApp::registerApps()
{
  registerApp(SolidPropertiesApp);
}

void
SolidPropertiesApp::registerAll(Factory & f, ActionFactory & af, Syntax & /* s */)
{
  Registry::registerObjectsTo(f, {"SolidPropertiesApp"});
  Registry::registerActionsTo(af, {"SolidPropertiesApp"});
}

void
SolidPropertiesApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"SolidPropertiesApp"});
}

void
SolidPropertiesApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"SolidPropertiesApp"});
}

void
SolidPropertiesApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("use registerAll instead of registerExecFlags");
}

extern "C" void
SolidPropertiesApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SolidPropertiesApp::registerAll(f, af, s);
}
extern "C" void
SolidPropertiesApp__registerApps()
{
  SolidPropertiesApp::registerApps();
}
