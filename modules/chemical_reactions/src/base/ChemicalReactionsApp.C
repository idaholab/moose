//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChemicalReactionsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<ChemicalReactionsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("ChemicalReactionsApp");

ChemicalReactionsApp::ChemicalReactionsApp(const InputParameters & parameters)
  : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ChemicalReactionsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ChemicalReactionsApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  ChemicalReactionsApp::registerExecFlags(_factory);
}

ChemicalReactionsApp::~ChemicalReactionsApp() {}

// External entry point for dynamic application loading
extern "C" void
ChemicalReactionsApp__registerApps()
{
  ChemicalReactionsApp::registerApps();
}
void
ChemicalReactionsApp::registerApps()
{
  registerApp(ChemicalReactionsApp);
}

// External entry point for dynamic object registration
extern "C" void
ChemicalReactionsApp__registerObjects(Factory & factory)
{
  ChemicalReactionsApp::registerObjects(factory);
}
void
ChemicalReactionsApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"ChemicalReactionsApp"});
}

// External entry point for dynamic syntax association
extern "C" void
ChemicalReactionsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  ChemicalReactionsApp::associateSyntax(syntax, action_factory);
}
void
ChemicalReactionsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"ChemicalReactionsApp"});
  registerSyntax("AddPrimarySpeciesAction", "ReactionNetwork/AqueousEquilibriumReactions");
  registerSyntax("AddPrimarySpeciesAction", "ReactionNetwork/SolidKineticReactions");
  registerSyntax("AddSecondarySpeciesAction", "ReactionNetwork/AqueousEquilibriumReactions");
  registerSyntax("AddSecondarySpeciesAction", "ReactionNetwork/SolidKineticReactions");
  registerSyntax("AddCoupledEqSpeciesAction", "ReactionNetwork/AqueousEquilibriumReactions");
  registerSyntax("AddCoupledSolidKinSpeciesAction", "ReactionNetwork/SolidKineticReactions");
}

// External entry point for dynamic execute flag registration
extern "C" void
ChemicalReactionsApp__registerExecFlags(Factory & factory)
{
  ChemicalReactionsApp::registerExecFlags(factory);
}
void
ChemicalReactionsApp::registerExecFlags(Factory & /*factory*/)
{
}
