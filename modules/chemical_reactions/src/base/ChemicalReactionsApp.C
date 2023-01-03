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

InputParameters
ChemicalReactionsApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

registerKnownLabel("ChemicalReactionsApp");

ChemicalReactionsApp::ChemicalReactionsApp(const InputParameters & parameters)
  : MooseApp(parameters)
{
  ChemicalReactionsApp::registerAll(_factory, _action_factory, _syntax);
}

ChemicalReactionsApp::~ChemicalReactionsApp() {}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerSyntax("AddPrimarySpeciesAction", "ReactionNetwork/AqueousEquilibriumReactions");
  registerSyntax("AddPrimarySpeciesAction", "ReactionNetwork/SolidKineticReactions");
  registerSyntax("AddSecondarySpeciesAction", "ReactionNetwork/AqueousEquilibriumReactions");
  registerSyntax("AddSecondarySpeciesAction", "ReactionNetwork/SolidKineticReactions");
  registerSyntax("AddCoupledEqSpeciesAction", "ReactionNetwork/AqueousEquilibriumReactions");
  registerSyntax("AddCoupledSolidKinSpeciesAction", "ReactionNetwork/SolidKineticReactions");
  registerSyntax("ChemicalCompositionAction", "ChemicalComposition");
}

void
ChemicalReactionsApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"ChemicalReactionsApp"});
  Registry::registerActionsTo(af, {"ChemicalReactionsApp"});
  associateSyntaxInner(s, af);
}

void
ChemicalReactionsApp::registerApps()
{
  registerApp(ChemicalReactionsApp);
}

void
ChemicalReactionsApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"ChemicalReactionsApp"});
}

void
ChemicalReactionsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"ChemicalReactionsApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
ChemicalReactionsApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

extern "C" void
ChemicalReactionsApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ChemicalReactionsApp::registerAll(f, af, s);
}

// External entry point for dynamic application loading
extern "C" void
ChemicalReactionsApp__registerApps()
{
  ChemicalReactionsApp::registerApps();
}
