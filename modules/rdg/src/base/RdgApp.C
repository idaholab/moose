//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RdgApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
RdgApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;

  return params;
}

registerKnownLabel("RdgApp");

RdgApp::RdgApp(const InputParameters & parameters) : MooseApp(parameters)
{
  RdgApp::registerAll(_factory, _action_factory, _syntax);
}

RdgApp::~RdgApp() {}

void
RdgApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  Registry::registerObjectsTo(f, {"RdgApp"});
  Registry::registerActionsTo(af, {"RdgApp"});
}

void
RdgApp::registerApps()
{
  registerApp(RdgApp);
}

extern "C" void
RdgApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  RdgApp::registerAll(f, af, s);
}
extern "C" void
RdgApp__registerApps()
{
  RdgApp::registerApps();
}
