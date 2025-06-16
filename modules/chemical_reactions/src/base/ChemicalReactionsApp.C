//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "Capabilities.h"

InputParameters
ChemicalReactionsApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;

  return params;
}

registerKnownLabel("ChemicalReactionsApp");

ChemicalReactionsApp::ChemicalReactionsApp(const InputParameters & parameters)
  : MooseApp(parameters)
{
  ChemicalReactionsApp::registerAll(_factory, _action_factory, _syntax);
}

ChemicalReactionsApp::~ChemicalReactionsApp() {}

void
ChemicalReactionsApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  Registry::registerObjectsTo(f, {"ChemicalReactionsApp"});
  Registry::registerActionsTo(af, {"ChemicalReactionsApp"});

  registerSyntax("AddPrimarySpeciesAction", "ReactionNetwork/AqueousEquilibriumReactions");
  registerSyntax("AddPrimarySpeciesAction", "ReactionNetwork/SolidKineticReactions");
  registerSyntax("AddSecondarySpeciesAction", "ReactionNetwork/AqueousEquilibriumReactions");
  registerSyntax("AddSecondarySpeciesAction", "ReactionNetwork/SolidKineticReactions");
  registerSyntax("AddCoupledEqSpeciesAction", "ReactionNetwork/AqueousEquilibriumReactions");
  registerSyntax("AddCoupledSolidKinSpeciesAction", "ReactionNetwork/SolidKineticReactions");
  registerSyntax("CommonChemicalCompositionAction", "ChemicalComposition");
  registerSyntax("ChemicalCompositionAction", "ChemicalComposition/*");
}

void
ChemicalReactionsApp::registerApps()
{
  const std::string doc = "Thermochimica Gibbs energy minimization library support ";
#ifdef THERMOCHIMICA_ENABLED
  addCapability("thermochimica", true, doc + "is available.");
#else
  addCapability("thermochimica", false, doc + "is not available.");
#endif

  registerApp(ChemicalReactionsApp);
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
