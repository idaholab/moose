//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeridynamicsApp.h"
#include "SolidMechanicsApp.h" // solid mechanics dependency
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
PeridynamicsApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;

  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;

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

  SolidMechanicsApp::registerApps();
}

void
PeridynamicsApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  Registry::registerObjectsTo(f, {"PeridynamicsApp"});
  Registry::registerActionsTo(af, {"PeridynamicsApp"});

  registerSyntax("MechanicsActionPD", "Modules/Peridynamics/Mechanics/Master/*");
  registerSyntax("GeneralizedPlaneStrainActionPD",
                 "Modules/Peridynamics/Mechanics/GeneralizedPlaneStrain/*");

  SolidMechanicsApp::registerAll(f, af, syntax);
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
