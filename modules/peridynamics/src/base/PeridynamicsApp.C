//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeridynamicsApp.h"
#include "TensorMechanicsApp.h" // tensor mechanics dependency
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<PeridynamicsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("PeridynamicsApp");

PeridynamicsApp::PeridynamicsApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  PeridynamicsApp::registerObjectDepends(_factory);
  PeridynamicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  PeridynamicsApp::associateSyntaxDepends(_syntax, _action_factory);
  PeridynamicsApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  PeridynamicsApp::registerExecFlags(_factory);
}

PeridynamicsApp::~PeridynamicsApp() {}

void
PeridynamicsApp::registerApps()
{
  registerApp(PeridynamicsApp);
}

void
PeridynamicsApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"PeridynamicsApp"});
}

void
PeridynamicsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"PeridynamicsApp"});

  registerSyntax("MechanicsActionPD", "Modules/Peridynamics/Mechanics");
  registerSyntax("GeneralizedPlaneStrainActionPD", "Modules/Peridynamics/GeneralizedPlaneStrain/*");
}

void
PeridynamicsApp::registerObjectDepends(Factory & factory)
{
  TensorMechanicsApp::registerObjects(factory);
}

void
PeridynamicsApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
}

void
PeridynamicsApp::registerExecFlags(Factory & /*factory*/)
{
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
PeridynamicsApp__registerApps()
{
  PeridynamicsApp::registerApps();
}

extern "C" void
PeridynamicsApp__registerObjects(Factory & factory)
{
  PeridynamicsApp::registerObjects(factory);
}

extern "C" void
PeridynamicsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  PeridynamicsApp::associateSyntax(syntax, action_factory);
}

extern "C" void
PeridynamicsApp__registerExecFlags(Factory & factory)
{
  PeridynamicsApp::registerExecFlags(factory);
}
