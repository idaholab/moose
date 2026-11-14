//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosTrainingApp.h"

#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

InputParameters
KokkosTrainingApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  return params;
}

KokkosTrainingApp::KokkosTrainingApp(const InputParameters & parameters) : MooseApp(parameters)
{
  KokkosTrainingApp::registerAll(_factory, _action_factory, _syntax);
}

void
KokkosTrainingApp::registerApps()
{
  registerApp(KokkosTrainingApp);
}

void
KokkosTrainingApp::registerAll(Factory & factory, ActionFactory & action_factory, Syntax & syntax)
{
  Registry::registerObjectsTo(factory, {"KokkosTrainingApp"});
  Registry::registerActionsTo(action_factory, {"KokkosTrainingApp"});
  ModulesApp::registerAllObjects<KokkosTrainingApp>(factory, action_factory, syntax);
}
