/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ChemicalReactionsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "PrimaryTimeDerivative.h"
#include "PrimaryConvection.h"
#include "PrimaryDiffusion.h"
#include "CoupledBEEquilibriumSub.h"
#include "CoupledConvectionReactionSub.h"
#include "CoupledDiffusionReactionSub.h"
#include "CoupledBEKinetic.h"
#include "DesorptionFromMatrix.h"
#include "DesorptionToPorespace.h"

#include "AqueousEquilibriumRxnAux.h"
#include "KineticDisPreConcAux.h"
#include "KineticDisPreRateAux.h"

#include "AddPrimarySpeciesAction.h"
#include "AddSecondarySpeciesAction.h"
#include "AddCoupledEqSpeciesKernelsAction.h"
#include "AddCoupledEqSpeciesAuxKernelsAction.h"
#include "AddCoupledSolidKinSpeciesKernelsAction.h"
#include "AddCoupledSolidKinSpeciesAuxKernelsAction.h"

#include "ChemicalOutFlowBC.h"

#include "LangmuirMaterial.h"
#include "MollifiedLangmuirMaterial.h"

// Initialize static member variables
bool ChemicalReactionsApp::_registered_objects = false;
bool ChemicalReactionsApp::_associated_syntax = false;

template<>
InputParameters validParams<ChemicalReactionsApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  return params;
}

ChemicalReactionsApp::ChemicalReactionsApp(const InputParameters & parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ChemicalReactionsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ChemicalReactionsApp::associateSyntax(_syntax, _action_factory);
}

ChemicalReactionsApp::~ChemicalReactionsApp()
{
}

// External entry point for dynamic application loading
extern "C" void ChemicalReactionsApp__registerApps() { ChemicalReactionsApp::registerApps(); }
void
ChemicalReactionsApp::registerApps()
{
  registerApp(ChemicalReactionsApp);
}

// External entry point for dynamic object registration
extern "C" void ChemicalReactionsApp__registerObjects(Factory & factory) { ChemicalReactionsApp::registerObjects(factory); }
void
ChemicalReactionsApp::registerObjects(Factory & factory)
{
  if (_registered_objects)
    return;
  _registered_objects = true;

  registerKernel(PrimaryTimeDerivative);
  registerKernel(PrimaryConvection);
  registerKernel(PrimaryDiffusion);
  registerKernel(CoupledBEEquilibriumSub);
  registerKernel(CoupledConvectionReactionSub);
  registerKernel(CoupledDiffusionReactionSub);
  registerKernel(CoupledBEKinetic);
  registerKernel(DesorptionFromMatrix);
  registerKernel(DesorptionToPorespace);

  registerAux(AqueousEquilibriumRxnAux);
  registerAux(KineticDisPreConcAux);
  registerAux(KineticDisPreRateAux);

  registerBoundaryCondition(ChemicalOutFlowBC);

  registerMaterial(LangmuirMaterial);
  registerMaterial(MollifiedLangmuirMaterial);
}

// External entry point for dynamic syntax association
extern "C" void ChemicalReactionsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { ChemicalReactionsApp::associateSyntax(syntax, action_factory); }
void
ChemicalReactionsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  if (_associated_syntax)
    return;
  _associated_syntax = true;

  syntax.registerActionSyntax("AddPrimarySpeciesAction", "ReactionNetwork");
  syntax.registerActionSyntax("AddSecondarySpeciesAction", "ReactionNetwork/AqueousEquilibriumReactions");
  syntax.registerActionSyntax("AddSecondarySpeciesAction", "ReactionNetwork/SolidKineticReactions");
  syntax.registerActionSyntax("AddCoupledEqSpeciesKernelsAction", "ReactionNetwork/AqueousEquilibriumReactions");
  syntax.registerActionSyntax("AddCoupledEqSpeciesAuxKernelsAction", "ReactionNetwork/AqueousEquilibriumReactions");
  syntax.registerActionSyntax("AddCoupledSolidKinSpeciesKernelsAction", "ReactionNetwork/SolidKineticReactions");
  syntax.registerActionSyntax("AddCoupledSolidKinSpeciesAuxKernelsAction", "ReactionNetwork/SolidKineticReactions");
  registerAction(AddPrimarySpeciesAction, "add_variable");
  registerAction(AddSecondarySpeciesAction, "add_aux_variable");
  registerAction(AddCoupledEqSpeciesKernelsAction, "add_kernel");
  registerAction(AddCoupledEqSpeciesAuxKernelsAction, "add_aux_kernel");
  registerAction(AddCoupledSolidKinSpeciesKernelsAction, "add_kernel");
  registerAction(AddCoupledSolidKinSpeciesAuxKernelsAction, "add_aux_kernel");
}
