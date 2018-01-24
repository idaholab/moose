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
#include "DarcyPressure.h"
#include "PackedColumn.h"
#include "DarcyVelocity.h"

// Moose Includes
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

template <>
InputParameters
validParams<DarcyThermoMechApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

DarcyThermoMechApp::DarcyThermoMechApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);
  DarcyThermoMechApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
  DarcyThermoMechApp::associateSyntax(_syntax, _action_factory);
}

void
DarcyThermoMechApp::registerApps()
{
  registerApp(DarcyThermoMechApp);
}

void
DarcyThermoMechApp::registerObjects(Factory & factory)
{
  registerKernel(DarcyPressure);
  registerMaterial(PackedColumn);
  registerAux(DarcyVelocity);
}

void
DarcyThermoMechApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
