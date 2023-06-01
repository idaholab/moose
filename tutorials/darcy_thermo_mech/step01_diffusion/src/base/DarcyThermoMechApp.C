//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Tutorial Includes
#include "DarcyThermoMechApp.h"

// MOOSE Includes
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

InputParameters
DarcyThermoMechApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

DarcyThermoMechApp::DarcyThermoMechApp(InputParameters parameters) : MooseApp(parameters)
{
  ModulesApp::registerAllObjects<DarcyThermoMechApp>(_factory, _action_factory, _syntax);
  Registry::registerObjectsTo(_factory, {"DarcyThermoMechApp"});
  Registry::registerActionsTo(_action_factory, {"DarcyThermoMechApp"});
}

void
DarcyThermoMechApp::registerApps()
{
  registerApp(DarcyThermoMechApp);
}
