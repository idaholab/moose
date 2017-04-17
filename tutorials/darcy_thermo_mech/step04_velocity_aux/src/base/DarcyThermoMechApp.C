/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
