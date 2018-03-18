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
#include "SolidMechanicsApp.h"
#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<XFEMApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}
registerKnownLabel("XFEMApp");

XFEMApp::XFEMApp(const InputParameters & parameters) : MooseApp(parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  XFEMApp::registerObjectDepends(_factory);
  XFEMApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  XFEMApp::associateSyntaxDepends(_syntax, _action_factory);
  XFEMApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  XFEMApp::registerExecFlags(_factory);
}

XFEMApp::~XFEMApp() {}

// External entry point for dynamic application loading
extern "C" void
XFEMApp__registerApps()
{
  XFEMApp::registerApps();
}
void
XFEMApp::registerApps()
{
  registerApp(XFEMApp);
}

void
XFEMApp::registerObjectDepends(Factory & factory)
{
  SolidMechanicsApp::registerObjects(factory);
  TensorMechanicsApp::registerObjects(factory);
}

// External entry point for dynamic object registration
extern "C" void
XFEMApp__registerObjects(Factory & factory)
{
  XFEMApp::registerObjects(factory);
}
void
XFEMApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"XFEMApp"});
}

void
XFEMApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  SolidMechanicsApp::associateSyntax(syntax, action_factory);
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
}

// External entry point for dynamic syntax association
extern "C" void
XFEMApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  XFEMApp::associateSyntax(syntax, action_factory);
}
void
XFEMApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"XFEMApp"});

  registerTask("setup_xfem", false);
  syntax.addDependency("setup_xfem", "setup_adaptivity");
  registerSyntax("XFEMAction", "XFEM");
}

// External entry point for dynamic execute flag registration
extern "C" void
XFEMApp__registerExecFlags(Factory & factory)
{
  XFEMApp::registerExecFlags(factory);
}
void
XFEMApp::registerExecFlags(Factory & factory)
{
  registerExecFlag(EXEC_XFEM_MARK);
}
