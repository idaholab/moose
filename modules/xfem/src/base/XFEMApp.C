//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMApp.h"
#include "XFEMAppTypes.h"
#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
XFEMApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}
registerKnownLabel("XFEMApp");

XFEMApp::XFEMApp(const InputParameters & parameters) : MooseApp(parameters)
{
  srand(processor_id());
  XFEMApp::registerAll(_factory, _action_factory, _syntax);
}

XFEMApp::~XFEMApp() {}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerTask("setup_xfem", false);
  syntax.addDependency("setup_xfem", "setup_adaptivity");
  registerSyntax("XFEMAction", "XFEM");
}

void
XFEMApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"XFEMApp"});
  Registry::registerActionsTo(af, {"XFEMApp"});
  associateSyntaxInner(s, af);

  TensorMechanicsApp::registerAll(f, af, s);
}

void
XFEMApp::registerApps()
{
  registerApp(XFEMApp);
}

void
XFEMApp::registerObjectDepends(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjectsDepends");
  TensorMechanicsApp::registerObjects(factory);
}

void
XFEMApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"XFEMApp"});
}

void
XFEMApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntaxDepends");
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
}

void
XFEMApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"XFEMApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
XFEMApp::registerExecFlags(Factory &)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

extern "C" void
XFEMApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  XFEMApp::registerAll(f, af, s);
}
extern "C" void
XFEMApp__registerApps()
{
  XFEMApp::registerApps();
}
