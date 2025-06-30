//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMApp.h"
#include "XFEMAppTypes.h"
#include "SolidMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
XFEMApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;

  return params;
}
registerKnownLabel("XFEMApp");

XFEMApp::XFEMApp(const InputParameters & parameters) : MooseApp(parameters)
{
  srand(processor_id());
  XFEMApp::registerAll(_factory, _action_factory, _syntax);
}

XFEMApp::~XFEMApp() {}

void
XFEMApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  Registry::registerObjectsTo(f, {"XFEMApp"});
  Registry::registerActionsTo(af, {"XFEMApp"});

  registerTask("setup_xfem", false);
  syntax.addDependency("setup_xfem", "setup_adaptivity");
  registerSyntax("XFEMAction", "XFEM");

  SolidMechanicsApp::registerAll(f, af, syntax);
}

void
XFEMApp::registerApps()
{
  registerApp(XFEMApp);

  SolidMechanicsApp::registerApps();
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
