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
#include "DarcyThermoMechApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

// Kernels
#include "DarcyPressure.h"
#include "DarcyConvection.h"

// BCs
#include "HeatConductionOutflow.h"

// Materials
#include "PackedColumn.h"

// AuxKernels
#include "DarcyVelocity.h"

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

DarcyThermoMechApp::~DarcyThermoMechApp() {}

void
DarcyThermoMechApp::registerApps()
{
  registerApp(DarcyThermoMechApp);
}

void
DarcyThermoMechApp::registerObjects(Factory & factory)
{
  registerKernel(DarcyPressure);
  registerKernel(DarcyConvection);

  registerMaterial(PackedColumn);
  registerAux(DarcyVelocity);
  registerBoundaryCondition(HeatConductionOutflow);
}

void
DarcyThermoMechApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
